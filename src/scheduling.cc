void scheduling_prefetch(){
    if (migration_policy_str=="DEEPUM")
    {
        for (int i = 0; i < kernel_list.size(); i++)
        {
            std::vector<Tensor*> required_tensor_N;
            kernel_list[(i+prefetch_degree)%kernel_list.size()].getRequiredTensors(required_tensor_N);
            for (int j = 0; j < required_tensor_N.size(); j++)
            {
                DataMovementHint pre_fetch(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, i, required_tensor_N[j]);
                movement_hints.push_back(pre_fetch);
            }
        }
        return;
    }

    
    long total_mem_size = memory_offset_intermediate + memory_offset_weights + tensor_list[0]->size_in_byte;
    int kernel_num = kernel_list.size();
    long GPU_total_mem = (long)(GPU_memory_size_GB * 1024 * 1024 * 1024);
    long target_mem_line = (long)(GPU_memory_size_GB * 1024 * 1024 * 1024) * 0.99;
    long tolerant_line = target_mem_line * 0.7; 
    long CPU_line = (long)(CPU_memory_line_GB * 1024 * 1024 * 1024);

    int hill_index = 0;
    long hill_mem = 0;
    double SSD_bandwidth = SSD_PCIe_bandwidth_GBps;

    //Fill gpu memory first with all the tensors
    GPU_free_memory_estimation.resize(kernel_num);
    GPU_resident_memory_estimation.resize(kernel_num);
    for (int i = 0; i < kernel_num; i++)
    {
        GPU_resident_memory_estimation[i] = total_mem_size;
    }

    //Fill cpu memory first with 0
    CPU_resident_memory_estimation.resize(kernel_num);
    for (int i = 0; i < kernel_num; i++)
    {
        CPU_resident_memory_estimation[i] = 0;
    }

    //Initialize the BW estimation arrays
    ssd2gpu_BW_estimation.resize(kernel_num);
    pcie2gpu_BW_estimation.resize(kernel_num);
    gpu2ssd_BW_estimation.resize(kernel_num);
    gpu2pcie_BW_estimation.resize(kernel_num);



    for (int i = 0; i < kernel_num; i++)
    {
        ssd2gpu_BW_estimation[i].capacity = SSD_bandwidth*1024*1024*1024/1000000*(kernel_time_table[i+1] - kernel_time_table[i]);
        ssd2gpu_BW_estimation[i].estimation = 0;
        ssd2gpu_BW_estimation[i].full = false;
        gpu2ssd_BW_estimation[i].capacity = SSD_bandwidth*1024*1024*1024/1000000*(kernel_time_table[i+1] - kernel_time_table[i]);
        gpu2ssd_BW_estimation[i].estimation = 0;
        gpu2ssd_BW_estimation[i].full = false;
        pcie2gpu_BW_estimation[i].capacity = CPU_PCIe_bandwidth_GBps*1024*1024*1024/1000000*(kernel_time_table[i+1] - kernel_time_table[i]);
        pcie2gpu_BW_estimation[i].estimation = 0;
        pcie2gpu_BW_estimation[i].full = false;
        gpu2pcie_BW_estimation[i].capacity = CPU_PCIe_bandwidth_GBps*1024*1024*1024/1000000*(kernel_time_table[i+1] - kernel_time_table[i]);
        gpu2pcie_BW_estimation[i].estimation = 0;
        gpu2pcie_BW_estimation[i].full = false;
    }


    if(migration_policy_str=="G10GDSSSD" || migration_policy_str=="G10GDSFULL"){ //For GDS-based baselines, do some loosen on the target line 
                                                                                 //can gain best performance since without UVM the GPU mem requirement is stict
        loosen_parameter = 1.060606;
        if (borden<200 && is_transformer==1 && migration_policy_str=="G10GDSFULL")
        {
            loosen_parameter = 1.269366;
        }
        
    }
    target_mem_line = loosen_parameter * target_mem_line;    

    //Except for A0, pre-deallocation all other tensors  #First pass - to figure out the memory pressure region
    for (int i = 1; i < tensor_list.size(); i++)
    {
        if (check_GPU_OK(target_mem_line))    //If already OK, end this loop
        {
            break;
        }
        
        Tensor* curr_tensor = tensor_list[i];
        if (!curr_tensor->is_global_weight)
        {
            //First do pre-alloc
            int issue_index;
            int birth_date_index = curr_tensor->live_interval[0];
            double estimated_pre_alloc_time = curr_tensor->size_in_byte * GPU_malloc_uspB;
            double pre_alloc_start_time_precise = kernel_time_table[birth_date_index] - estimated_pre_alloc_time;
            if (pre_alloc_start_time_precise < 0)
            {
                //DataMovementHint pre_allo(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, 0, curr_tensor);
                //movement_hints.push_back(pre_allo);
                issue_index = 0;

                //minus mem
                for (int j = 0; j < birth_date_index; j++)
                {
                    GPU_resident_memory_estimation[j] -= curr_tensor->size_in_byte;
                }
            }
            else
            {
                    
                for (int j = 0; j < birth_date_index; j++)
                {
                    if (kernel_time_table[j] <= pre_alloc_start_time_precise && kernel_time_table[j+1] > pre_alloc_start_time_precise)
                    {
                        //DataMovementHint pre_allo(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, j, curr_tensor);
                        //movement_hints.push_back(pre_allo);
                        issue_index = j;
                        break;
                    }
                }
                
                //minus mem
                for (int j = 0; j < issue_index; j++)
                {
                   GPU_resident_memory_estimation[j] -= curr_tensor->size_in_byte;
                }
                    
            }


            //Second do pre-deallocation
            int death_index = curr_tensor->live_interval[1];
            if (curr_tensor->live_interval[1]==-1)
            {
                death_index = curr_tensor->live_interval[0] + 1;
            }
            
            //DataMovementHint pre_dallo(PageLocation::NOT_KNOWN, PageLocation::NOT_PRESENT, death_index, curr_tensor);
            //movement_hints.push_back(pre_dallo);

            //double deallo_time = curr_tensor->size_in_byte * GPU_free_uspB;
            double deallo_time = 0;
            double deallo_finish_time_precise = kernel_time_table[death_index];
            if (deallo_finish_time_precise < kernel_time_table[kernel_num])
            {
                int finish_index = -1;
                for (int j = death_index; j < kernel_num; j++)
                {
                    if (kernel_time_table[j] <= deallo_finish_time_precise && kernel_time_table[j+1] > deallo_finish_time_precise)
                    {
                        finish_index = j;
                        break;
                    }
                }
                //Assert(finish_index >= 0);
                if (finish_index == -1)
                {
                    finish_index = kernel_index;
                }
                

                //minus mem
                for (int j = finish_index; j < kernel_num; j++)
                {
                    GPU_resident_memory_estimation[j] -= curr_tensor->size_in_byte;
                }
            }
        }

    
    }

    bool is_under_pressure = false;
    int pressure_region[2]; 
    pressure_region[0] = -1;
    pressure_region[1] = -1;
    if (!check_GPU_OK(target_mem_line))    //If already OK, end this loop
    {
        is_under_pressure = true;
    }
    if (is_under_pressure)
    {
        int k_index = 0;
        while (k_index < kernel_num)
        {
            if (GPU_resident_memory_estimation[k_index] > target_mem_line)
            {
                pressure_region[0] = k_index;
                break;
            }
            k_index++;
        }
        k_index = kernel_num - 1;
        while (k_index >= 0)
        {
            if (GPU_resident_memory_estimation[k_index] > target_mem_line)
            {
                pressure_region[1] = k_index;
                break;
            }
            k_index--;
        }
        Assert(pressure_region[0] >= 0 && "Pressure region[0] is higher than 0-3043");
        Assert(pressure_region[1] >= 0 && "Pressure region[1] is higher than 0-3044");
        Assert(pressure_region[1] >= pressure_region[0] && "Pressure region[1] is higher than [0]-3045");
    }


    //Refill gpu memory with all the tensors
    GPU_resident_memory_estimation.resize(kernel_num);
    for (int i = 0; i < kernel_num; i++)
    {
        GPU_resident_memory_estimation[i] = total_mem_size;
    }
    


    //Except for A0, pre-deallocation all other tensors - Second pass, schedule the smart migration instructions
    for (int i = 1; i < tensor_list.size(); i++)
    {
        if (check_GPU_OK(target_mem_line))    //If already OK, end this loop
        {
            break;
        }
        
        Tensor* curr_tensor = tensor_list[i];
        if (!curr_tensor->is_global_weight)
        {
            //First do pre-alloc
            int issue_index;
            int birth_date_index = curr_tensor->live_interval[0];
            double estimated_pre_alloc_time;
            if (is_under_pressure && birth_date_index >= pressure_region[0] && birth_date_index <= pressure_region[1])
            {
                estimated_pre_alloc_time = curr_tensor->size_in_byte * GPU_malloc_uspB;
                //estimated_pre_alloc_time = estimated_pre_alloc_time * (1 + delta_parameter);
            }
            else
            {
                estimated_pre_alloc_time = curr_tensor->size_in_byte * GPU_malloc_uspB;
            }
            
            double pre_alloc_start_time_precise = kernel_time_table[birth_date_index] - estimated_pre_alloc_time;
            if (pre_alloc_start_time_precise < 0)
            {
                if (migration_policy_str!="G10GDSSSD" && migration_policy_str!="G10GDSFULL")
                {
                    DataMovementHint pre_allo(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, 0, curr_tensor);
                    movement_hints.push_back(pre_allo);
                }
                issue_index = 0;

            }
            else
            {
                    
                for (int j = 0; j < birth_date_index; j++)
                {
                    if (kernel_time_table[j] <= pre_alloc_start_time_precise && kernel_time_table[j+1] > pre_alloc_start_time_precise)
                    {
                        if (migration_policy_str!="G10GDSSSD" && migration_policy_str!="G10GDSFULL"){
                            DataMovementHint pre_allo(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, j, curr_tensor);
                            movement_hints.push_back(pre_allo);
                        }
                        issue_index = j;
                        break;
                    }
                }
                
                //minus mem
                for (int j = 0; j < issue_index; j++)
                {
                   GPU_resident_memory_estimation[j] -= curr_tensor->size_in_byte;
                }
                    
            }


            //Second do pre-deallocation
            int death_index = curr_tensor->live_interval[1];
            if (curr_tensor->live_interval[1]==-1)
            {
                death_index = curr_tensor->live_interval[0] + 1;
            }
            
            if (migration_policy_str!="G10GDSSSD" && migration_policy_str!="G10GDSFULL"){
                DataMovementHint pre_dallo(PageLocation::NOT_KNOWN, PageLocation::NOT_PRESENT, death_index, curr_tensor);
                movement_hints.push_back(pre_dallo);
            }

            //double deallo_time = curr_tensor->size_in_byte * GPU_free_uspB;
            double deallo_time = 0;
            double deallo_finish_time_precise = kernel_time_table[death_index];
            if (deallo_finish_time_precise < kernel_time_table[kernel_num])
            {
                int finish_index = -1;
                for (int j = death_index; j < kernel_num; j++)
                {
                    if (kernel_time_table[j] <= deallo_finish_time_precise && kernel_time_table[j+1] > deallo_finish_time_precise)
                    {
                        finish_index = j;
                        break;
                    }
                }
                //Assert(finish_index >= 0);
                if (finish_index == -1)
                {
                    finish_index = kernel_index;
                }
                

                //minus mem
                for (int j = finish_index; j < kernel_num; j++)
                {
                    GPU_resident_memory_estimation[j] -= curr_tensor->size_in_byte;
                }
            }
        }

    
    }



    
    std::cout<<"After pre-deallocation"<<std::endl;
    print_GPU_mem_estimation();

    for (int j = 0; j < GPU_resident_memory_estimation.size(); j++)
    {
        if (GPU_resident_memory_estimation[j] > hill_mem)
        {
            hill_mem = GPU_resident_memory_estimation[j];
            hill_index = j;
        }
    }
    

    //Fill the looped extend kernel time table      0 - 2 * kernel_num
    std::vector<double> kernel_time_table_extended;
    kernel_time_table_extended.resize(kernel_num);
    for (int j = 0; j < kernel_num; j++)
    {
        kernel_time_table_extended[j] = kernel_time_table[j];
    }
    double last_time = kernel_time_table[kernel_num];
    kernel_time_table_extended.push_back(last_time);
    for (int j = 0; j < kernel_num; j++)
    {
        last_time += (double)kernel_list[j].execution_cycles / (double)(GPU_frequency_GHz*1000);
        kernel_time_table_extended.push_back(last_time);
    }


    //Schedule the prefetches and pre-evictions, Go through all the intervals, from largest to shortest
    int cold_period_iter = 0;
    int tot_iter_num = interval_list.size();

    for (int i = 0; i < tot_iter_num; i++)
    {
        if (check_GPU_OK(target_mem_line))    //If already OK, end this loop
        {
            cold_period_iter = i;
            break;
        }
        else if ( i == interval_list.size()-1)
        {
            cold_period_iter = interval_list.size();
        }

        //Sort the intervals
        std::sort(interval_list.begin(), interval_list.end(), [](Hidding_Interval* a, Hidding_Interval* b){
            double area_can_reduce_a = 0;
            double area_can_reduce_b = 0;

            if (!a->is_looped)
            {
                int offload_mid_index;
                int prefetch_mid_index;
                offload_mid_index = gpu2ssd_BWgiveIndx_half(a->the_tensor->size_in_byte, a->kernelLevel_interval[0]);
                prefetch_mid_index = ssd2gpu_BWgiveIndx_half(a->the_tensor->size_in_byte, a->kernelLevel_interval[1]);

                for (int j = offload_mid_index; j < prefetch_mid_index; j++)
                {
                    if (GPU_resident_memory_estimation[j] > a->GPU_mem_line)
                    {
                        area_can_reduce_a += a->the_tensor->size_in_byte * (kernel_time_table[j+1] - kernel_time_table[j]);
                    }
                }
            }
            else
            {
                for (int j = a->kernelLevel_interval[0]; j < a->kernelLevel_interval[1] + kernel_list.size(); j++)
                {
                    if (GPU_resident_memory_estimation[j%kernel_list.size()] > a->GPU_mem_line)
                    {
                        area_can_reduce_a += a->the_tensor->size_in_byte * (kernel_time_table[(j+1)%(kernel_list.size()+1)] - kernel_time_table[j%(kernel_list.size()+1)]);
                    }
                }
            }

            if (!b->is_looped)
            {
                int offload_mid_index;
                int prefetch_mid_index;
                offload_mid_index = gpu2ssd_BWgiveIndx_half(b->the_tensor->size_in_byte, b->kernelLevel_interval[0]);
                prefetch_mid_index = ssd2gpu_BWgiveIndx_half(b->the_tensor->size_in_byte, b->kernelLevel_interval[1]);

                for (int j = offload_mid_index; j < prefetch_mid_index; j++)
                {
                    if (GPU_resident_memory_estimation[j] > b->GPU_mem_line)
                    {
                        area_can_reduce_b += b->the_tensor->size_in_byte * (kernel_time_table[j+1] - kernel_time_table[j]);
                    }
                }
            }
            else
            {
                for (int j = b->kernelLevel_interval[0]; j < b->kernelLevel_interval[1] + kernel_list.size(); j++)
                {
                    if (GPU_resident_memory_estimation[j%kernel_list.size()] > b->GPU_mem_line)
                    {
                        area_can_reduce_b += b->the_tensor->size_in_byte * (kernel_time_table[(j+1)%(kernel_list.size()+1)] - kernel_time_table[j%(kernel_list.size()+1)]);
                    }
                }
            }

            if (a->is_offloaded)
            {
                area_can_reduce_a = 0;
            }
            if (b->is_offloaded)
            {
                area_can_reduce_b = 0;
            }
            
            if ((area_can_reduce_a - area_can_reduce_b < 0.000001) && (area_can_reduce_a - area_can_reduce_b > -0.000001) && area_can_reduce_a != 0 && area_can_reduce_b != 0)
            {
                return (a->time_estimated * a->the_tensor->size_in_byte) > (b->time_estimated * b->the_tensor->size_in_byte);
            }
            else 
            {
                return (area_can_reduce_a > area_can_reduce_b);
            }
        });


        
        // The interval list is already sorted
        Hidding_Interval* curr_interval = interval_list[0];
        curr_interval->print();
        if (curr_interval->is_offloaded)
        {
            break;
        }
        
        if (check_GPU_OK_interval(target_mem_line, curr_interval->kernelLevel_interval[0], curr_interval->kernelLevel_interval[1]))
        {
            curr_interval->is_offloaded = true;
            continue;
        }

        int cha;
        if (!curr_interval->is_looped)
        {
            cha = curr_interval->kernelLevel_interval[1] - curr_interval->kernelLevel_interval[0];
        }
        else
        {
            cha = curr_interval->kernelLevel_interval[1] + kernel_num - curr_interval->kernelLevel_interval[0];
        }
        if (cha==1)
        {
            curr_interval->is_offloaded = true;
            continue;
        }



        double ssd_safe_time = 2*(SSD_latency_us + system_latency_us + curr_interval->the_tensor->size_in_byte / (double)(SSD_bandwidth*1024*1024*1024/1000000));
        double ssd_movement_estimated_time = ssd_safe_time / 2;
        double delta_time = delta_parameter * ssd_movement_estimated_time; //TODO: Very Important, we need to Prune this
        ssd_safe_time += delta_time;
        double ssd_prefetch_estimated_time = ssd_movement_estimated_time + delta_time;

        double cpu_safe_time = 2*(system_latency_us + curr_interval->the_tensor->size_in_byte / (double)(CPU_PCIe_bandwidth_GBps*1024*1024*1024/1000000));
        double cpu_movement_estimated_time = cpu_safe_time / 2;
        double delta_cpu_time = delta_parameter * cpu_movement_estimated_time; //TODO: Very Important, we need to Prune this
        cpu_safe_time += delta_cpu_time;
        double cpu_prefetch_estimated_time = cpu_movement_estimated_time + delta_cpu_time;
        
        // Check if the oversubscription occurs
        int the_index = curr_interval->kernelLevel_interval[0]; // See if the oversubscription occurs at the scheduling time
        long current_mem = GPU_total_mem - GPU_resident_memory_estimation[the_index];
        if (current_mem < 0)
        {
            current_mem = 0;
        }
        double over_movement_time = 0;

        long diff = current_mem - curr_interval->the_tensor->size_in_byte;
        std::cout<<"Mem-tensor"<<": "<<diff<<std::endl;
        
        // oversubscription inspection

        if (current_mem - curr_interval->the_tensor->size_in_byte < 0)
        {
            std::cout<<"oversubscribed index"<<": "<<the_index<<std::endl;
            long over_size = curr_interval->the_tensor->size_in_byte - current_mem;
            if (over_size < 0)
            {
                over_size = over_size * (-1);
            }
            std::cout<<"oversubscribed size"<<": "<<over_size<<std::endl;
            over_movement_time = 2*(SSD_latency_us + system_latency_us + over_size / (double)(SSD_PCIe_bandwidth_GBps*1024*1024*1024/1000000));
            std::cout<<"interval time"<<": "<<curr_interval->time_estimated<<std::endl;
            std::cout<<"Estimated cost"<<": "<<ssd_safe_time - over_movement_time<<std::endl;
            std::cout<<"SSD movement time"<<": "<<ssd_safe_time<<std::endl;
            std::cout<<"Oversubscription cost"<<": "<<over_movement_time<<std::endl;
            
            if(ssd_safe_time - over_movement_time < 0){
                std::cout<<"Scheduled False"<<std::endl;
            }
            else{
                std::cout<<"Scheduled True"<<std::endl;
            }
        }

        //if (curr_interval->time_estimated > ssd_safe_time)
        //if (curr_interval->time_estimated > ssd_safe_time || GPU_resident_memory_estimation[the_index] > GPU_total_mem)
        if (curr_interval->time_estimated > ssd_safe_time - over_movement_time)
        {   
            
            
            if (!curr_interval->is_looped)  //Not looped
            {   
                //std::cout<<"Looped tensor"<<std::endl;
                std::cout<<"tensor name "<<": "<<curr_interval->the_tensor->tensor_id<<std::endl;
                std::cout<<"-----------------------------------"<<std::endl;

                //Find the  ideal  finished(clear) index
                double eviction_finish_time = kernel_time_table[curr_interval->kernelLevel_interval[0]] + ssd_movement_estimated_time;
                int eviction_clear_index = -1;
                for (int j = curr_interval->kernelLevel_interval[0]; j < curr_interval->kernelLevel_interval[1]; j++)
                {
                    if (kernel_time_table[j] <= eviction_finish_time && kernel_time_table[j+1] > eviction_finish_time)
                    {
                        eviction_clear_index = j;
                        break;
                    }
                }
                Assert(eviction_clear_index >= 0 && "Eviction clear index is higher than 0");
                std::cout<<"eviction clear index "<<": "<<eviction_clear_index<<std::endl;

                //NEW: Use PCIe estimation to get the finishing index
                int pcie_eviction_clear_index = -1;
                pcie_eviction_clear_index = gpu2ssd_BWcheck(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0], eviction_clear_index);
                std::cout<<"pcie eviction clear index "<<": "<<pcie_eviction_clear_index<<std::endl;

                //Second calculate the prefetch ideal index
                // double prefetch_start_time_precise = kernel_time_table[curr_interval->kernelLevel_interval[1]] - ssd_prefetch_estimated_time;
                // int prefetch_start_index = -1;
                // for (int j = curr_interval->kernelLevel_interval[0]; j < curr_interval->kernelLevel_interval[1]; j++)
                // {
                //     if (kernel_time_table[j] <= prefetch_start_time_precise && kernel_time_table[j+1] > prefetch_start_time_precise)
                //     {
                //         prefetch_start_index = j;
                //         break;
                //     }
                // }
                // Assert(prefetch_start_index>=0);

                int pcie_prefetch_index = -1;



                if (pcie_eviction_clear_index==-1 && check_CPU_OK(CPU_line - curr_interval->the_tensor->size_in_byte)) //SSD is not OK, CPU is OK
                {
                    //Calculate cpu-prefetch-index
                    //pcie2gpu_BWsim
                    std::cout<<"Running PCIE Sim"<<std::endl;

                    pcie_prefetch_index = pcie2gpu_BWgiveIndx(curr_interval->the_tensor->size_in_byte*1.0, curr_interval->kernelLevel_interval[1]);
                    Assert(pcie_prefetch_index >=0 && "Prefetch clear index is higher than 0-3411");
                    // Get cpu eviction finish index
                    pcie_eviction_clear_index = gpu2pcie_BWgiveIndx(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0]);
                    Assert(pcie_eviction_clear_index >=0 && "PCIE eviction clear index is higher than 0-3414");
                    
                    
                    if(pcie_prefetch_index > pcie_eviction_clear_index){
                        //First schedule the pre-eviction
                        DataMovementHint pre_evict(PageLocation::NOT_KNOWN, PageLocation::IN_CPU, curr_interval->kernelLevel_interval[0], curr_interval->the_tensor);
                        pre_evict.human_readable_hint = "Preevict";
                        movement_hints.push_back(pre_evict);
                        curr_interval->the_tensor->is_choosed_to_evict = true;
                        curr_interval->is_really_offloaded = true;

                        // DataMovementHint pre_fetch(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, pcie_prefetch_index, curr_interval->the_tensor);
                        // movement_hints.push_back(pre_fetch);
                        curr_interval->original_prefetch_index = pcie_prefetch_index;
                        curr_interval->evict_finish_index = pcie_eviction_clear_index;
                        offloeded_local_intervals.push_back(curr_interval);

                        pcie2gpu_BWsim(curr_interval->the_tensor->size_in_byte, pcie_prefetch_index);
                        gpu2pcie_BWsim(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0]);
                        CPU_add_update_interval(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0], curr_interval->kernelLevel_interval[1]);

                    }
                    else{
                        curr_interval->is_offloaded  = true;
                        continue;
                    }
                }
                else
                {   
                    std::cout<<"Running SSD Sim"<<std::endl;
                    std::cout<<"tensor name in sim "<<": "<<curr_interval->the_tensor->tensor_id<<std::endl;
                    //Calculate ssd-prefetch-index
                    pcie_prefetch_index = ssd2gpu_BWgiveIndx(curr_interval->the_tensor->size_in_byte*1.0, curr_interval->kernelLevel_interval[1]);
                    Assert(pcie_prefetch_index >=0 && "PCIE prefetch index is higher than 0- 3444");
                    std::cout<<"pcie prefetch index"<<": "<<pcie_prefetch_index<<std::endl;
                    // Get cpu eviction finish index
                    pcie_eviction_clear_index = gpu2ssd_BWgiveIndx(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0]);
                    Assert(pcie_eviction_clear_index >=0 && "PCIE eviction clear index is higher than 0-3447");
                    std::cout<<"pcie eviction clear idx with simulator"<<": "<<pcie_eviction_clear_index<<std::endl;

                    // implementation 1
                    // Here we revise this if statement as 
                    // prefetching latency + pre-eviction latency - inactive interval < LRU-based evcition latency for the over_size + fetch latency for the over_size

                    if(pcie_prefetch_index < pcie_eviction_clear_index){
                    
                        // long over_size = curr_interval->the_tensor->size_in_byte - current_mem;
                        long over_size = curr_interval->the_tensor->size_in_byte;
                        // if (over_size < 0)
                        // {
                        //     over_size = over_size * (-1);
                        // }
                        std::cout<<"oversubscribed size"<<": "<<over_size<<std::endl;
                        over_movement_time = 2*(SSD_latency_us + system_latency_us + over_size / (double)(SSD_PCIe_bandwidth_GBps*1024*1024*1024/1000000));
                        
                        long delayed_prefetch_latency = kernel_time_table_extended[pcie_eviction_clear_index] - kernel_time_table_extended[pcie_prefetch_index];
                        std::cout<<"Saved time"<<": "<<over_movement_time - delayed_prefetch_latency<<std::endl;

                        if(over_movement_time - delayed_prefetch_latency > 0){
                            pcie_prefetch_index = pcie_eviction_clear_index +1;
                            std::cout<<"Rescheduled True"<<std::endl;
                        }
            
                    }
                    
                    if(pcie_prefetch_index > pcie_eviction_clear_index){
                        //First schedule the pre-eviction
                        DataMovementHint pre_evict(PageLocation::NOT_KNOWN, PageLocation::IN_SSD, curr_interval->kernelLevel_interval[0], curr_interval->the_tensor);
                        pre_evict.human_readable_hint = "Preevict";
                        movement_hints.push_back(pre_evict);
                        curr_interval->the_tensor->is_choosed_to_evict = true;
                        curr_interval->is_really_offloaded = true;

                        // DataMovementHint pre_fetch(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, pcie_prefetch_index, curr_interval->the_tensor);
                        // movement_hints.push_back(pre_fetch);
                        curr_interval->original_prefetch_index = pcie_prefetch_index;
                        curr_interval->evict_finish_index = pcie_eviction_clear_index;
                        offloeded_local_intervals.push_back(curr_interval);

                        ssd2gpu_BWsim(curr_interval->the_tensor->size_in_byte, pcie_prefetch_index);
                        gpu2ssd_BWsim(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0]);
                    }
                    else{
                        curr_interval->is_offloaded = true;
                        continue;
                    }
                }
                
                //minus mem
                Assert(pcie_eviction_clear_index>=0 && "PCIE eviction clear index is higher than 0-3473");
                for (int j = pcie_eviction_clear_index + 1; j < pcie_prefetch_index; j++)
                {
                    GPU_resident_memory_estimation[j] -= curr_interval->the_tensor->size_in_byte;
                }
                curr_interval->is_offloaded = true;
            }
            else
            {    
                std::cout<<"tensor name in sim"<<": "<<curr_interval->the_tensor->tensor_id<<std::endl;
                std::cout<<"-----------------------------------"<<std::endl;

                //Find the finished(clear) index
                double eviction_finish_time = kernel_time_table[curr_interval->kernelLevel_interval[0]] + ssd_movement_estimated_time;
                int eviction_clear_index = -1;
                for (int j = curr_interval->kernelLevel_interval[0]; j < curr_interval->kernelLevel_interval[1] + kernel_num; j++)  // j is the extended table index
                {
                    if (kernel_time_table_extended[j] <= eviction_finish_time && kernel_time_table_extended[j+1] > eviction_finish_time)
                    {
                        eviction_clear_index = j;
                        break;
                    }
                }
                Assert(eviction_clear_index >= 0 && "Eviction clear index is higher than 0 - 3496");
                std::cout<<"eviction clear index "<<": "<<eviction_clear_index<<std::endl;

                //Second schedule the prefetch
                double prefetch_start_time_precise = kernel_time_table_extended[curr_interval->kernelLevel_interval[1] + kernel_num] - ssd_prefetch_estimated_time;
                int prefetch_start_index = -1;
                for (int j = curr_interval->kernelLevel_interval[0]; j < curr_interval->kernelLevel_interval[1] + kernel_num; j++)
                {
                    if (kernel_time_table_extended[j] <= prefetch_start_time_precise && kernel_time_table_extended[j+1] > prefetch_start_time_precise)
                    {
                        prefetch_start_index = j;
                        break;
                    }
                }
                Assert(prefetch_start_index>=0 && "Prefetch start index is higher than 0-3510");
                std::cout<<"prefetch start index "<<": "<<prefetch_start_index<<std::endl;
                if (prefetch_start_index!=curr_interval->kernelLevel_interval[0])
                {
                    //First schedule the pre-eviction
                    DataMovementHint pre_evict(PageLocation::NOT_KNOWN, PageLocation::IN_SSD, curr_interval->kernelLevel_interval[0], curr_interval->the_tensor);
                    pre_evict.human_readable_hint = "Preevict";
                    movement_hints.push_back(pre_evict);
                    curr_interval->the_tensor->is_choosed_to_evict = true;
                    curr_interval->is_really_offloaded = true;

                    DataMovementHint pre_fetch(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, prefetch_start_index % kernel_num, curr_interval->the_tensor);
                    pre_fetch.human_readable_hint = "Prefetch";
                    movement_hints.push_back(pre_fetch);
                }
                

                //minus mem
                for (int j = eviction_clear_index + 1; j < prefetch_start_index; j++)
                {
                    GPU_resident_memory_estimation[(j%kernel_num)] -= curr_interval->the_tensor->size_in_byte;
                }
                curr_interval->is_offloaded = true;
            }
            
            
        }
        else if (curr_interval->time_estimated > cpu_safe_time)
        {
            /* code */
            if (!check_CPU_OK(CPU_line - curr_interval->the_tensor->size_in_byte))    //If already full, end this loop iteration
            {
                curr_interval->is_offloaded = true;
                continue;
            }


            if (!curr_interval->is_looped)  //Not looped
            {
                
                //Find the finished(clear) index
                // double eviction_finish_time = kernel_time_table[curr_interval->kernelLevel_interval[0]] + cpu_movement_estimated_time;
                // int eviction_clear_index = -1;
                // for (int j = curr_interval->kernelLevel_interval[0]; j < curr_interval->kernelLevel_interval[1]; j++)
                // {
                //     if (kernel_time_table[j] <= eviction_finish_time && kernel_time_table[j+1] > eviction_finish_time)
                //     {
                //         eviction_clear_index = j;
                //         break;
                //     }
                // }
                // Assert(eviction_clear_index >= 0);

                //NEW: Use PCIe estimation to get the finishing index
                int pcie_eviction_clear_index = -1;
                pcie_eviction_clear_index = gpu2pcie_BWgiveIndx(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0]);
                Assert(pcie_eviction_clear_index>=0 && "PCIE eviction clear index is higher than 0 - 3566");


                //Second schedule the prefetch
                // double prefetch_start_time_precise = kernel_time_table[curr_interval->kernelLevel_interval[1]] - cpu_prefetch_estimated_time;
                // int prefetch_start_index = -1;
                // for (int j = curr_interval->kernelLevel_interval[0]; j < curr_interval->kernelLevel_interval[1]; j++)
                // {
                //     if (kernel_time_table[j] <= prefetch_start_time_precise && kernel_time_table[j+1] > prefetch_start_time_precise)
                //     {
                //         prefetch_start_index = j;
                //         break;
                //     }
                // }
                // Assert(prefetch_start_index>=0);

                int pcie_prefetch_index = -1;
                pcie_prefetch_index = pcie2gpu_BWgiveIndx(curr_interval->the_tensor->size_in_byte*1.0, curr_interval->kernelLevel_interval[1]);
                Assert(pcie_prefetch_index >=0 && "PCIE prefetch is higher than 0 - 3584");


                if (pcie_prefetch_index > pcie_eviction_clear_index)
                {
                    //First schedule the pre-eviction
                    DataMovementHint pre_evict(PageLocation::NOT_KNOWN, PageLocation::IN_CPU, curr_interval->kernelLevel_interval[0], curr_interval->the_tensor);
                    pre_evict.human_readable_hint = "Preevict";
                    movement_hints.push_back(pre_evict);
                    curr_interval->the_tensor->is_choosed_to_evict = true;
                    curr_interval->is_really_offloaded = true;

                    // DataMovementHint pre_fetch(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, pcie_prefetch_index, curr_interval->the_tensor);
                    // movement_hints.push_back(pre_fetch);
                    curr_interval->original_prefetch_index = pcie_prefetch_index;
                    curr_interval->evict_finish_index = pcie_eviction_clear_index;
                    offloeded_local_intervals.push_back(curr_interval);

                    pcie2gpu_BWsim(curr_interval->the_tensor->size_in_byte, pcie_prefetch_index);
                    gpu2pcie_BWsim(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0]);
                }

                //minus mem
                Assert(pcie_eviction_clear_index>=0 && "PCIE clear index is higher than 0-3607");
                for (int j = pcie_eviction_clear_index + 1; j < pcie_prefetch_index; j++)
                {
                    GPU_resident_memory_estimation[j] -= curr_interval->the_tensor->size_in_byte;
                }
                curr_interval->is_offloaded = true;
            }
            else
            {

                
                //Find the finished(clear) index
                double eviction_finish_time = kernel_time_table[curr_interval->kernelLevel_interval[0]] + cpu_movement_estimated_time;
                int eviction_clear_index = -1;
                for (int j = curr_interval->kernelLevel_interval[0]; j < curr_interval->kernelLevel_interval[1] + kernel_num; j++)  // j is the extended table index
                {
                    if (kernel_time_table_extended[j] <= eviction_finish_time && kernel_time_table_extended[j+1] > eviction_finish_time)
                    {
                        eviction_clear_index = j;
                        break;
                    }
                }
                Assert(eviction_clear_index >= 0 && "Eviction clear index is higher than 0-3629");


                //Second schedule the prefetch
                double prefetch_start_time_precise = kernel_time_table_extended[curr_interval->kernelLevel_interval[1] + kernel_num] - cpu_prefetch_estimated_time;
                int prefetch_start_index = -1;
                for (int j = curr_interval->kernelLevel_interval[0]; j < curr_interval->kernelLevel_interval[1] + kernel_num; j++)
                {
                    if (kernel_time_table_extended[j] <= prefetch_start_time_precise && kernel_time_table_extended[j+1] > prefetch_start_time_precise)
                    {
                        prefetch_start_index = j;
                        break;
                    }
                }
                Assert(prefetch_start_index>=0 && "Prefetch start index is higher than 0-3643");

                if (prefetch_start_index!=curr_interval->kernelLevel_interval[0])
                {
                    //First schedule the pre-eviction
                    DataMovementHint pre_evict(PageLocation::NOT_KNOWN, PageLocation::IN_CPU, curr_interval->kernelLevel_interval[0], curr_interval->the_tensor);
                    pre_evict.human_readable_hint = "Preevict";
                    movement_hints.push_back(pre_evict);
                    curr_interval->the_tensor->is_choosed_to_evict = true;
                    curr_interval->is_really_offloaded = true;

                    DataMovementHint pre_fetch(PageLocation::NOT_KNOWN, PageLocation::IN_GPU, prefetch_start_index % kernel_num, curr_interval->the_tensor);
                    pre_fetch.human_readable_hint = "Prefetch";
                    movement_hints.push_back(pre_fetch);
                }
                

                //minus mem
                for (int j = eviction_clear_index + 1; j < prefetch_start_index; j++)
                {
                    GPU_resident_memory_estimation[(j%kernel_num)] -= curr_interval->the_tensor->size_in_byte;
                }
                curr_interval->is_offloaded = true;
            }

            CPU_add_update_interval(curr_interval->the_tensor->size_in_byte, curr_interval->kernelLevel_interval[0], curr_interval->kernelLevel_interval[1]);

        }        
        

    }