a = [1,1,2,3,17,22,40,41,42,57,78,79,80,95,116,117,118,133,136,154,155,156,192,193,194,213,481,484,486,488,491,507,510,513,516,519,522,538,541,553,556,559,562,578,581,584,587,590,593,609,612,624,627,630,633,649,652,655,658,661,664,680,683,695,698,701,704,720,723,726,729,732,735,751,754,766,769,772,791,794,797,800,803,806,822,825,837,840,843,846,862,865,868,871,874,877,893,896,908,911,914,917,933,936,939,942,945,948,964,967]
b = [1,1,2,3,17,22,40,41,42,57,78,79,80,95,116,117,118,133,154,155,156,192,193,194,213,481,484,486,488,491,507,510,513,516,519,522,538,541,553,556,559,562,578,581,584,587,590,593,609,612,624,627,630,633,649,652,655,658,661,664,680,683,695,698,701,704,720,723,726,729,732,735,751,754,766,769,772,791,794,797,800,803,806,822,825,837,840,843,846,862,865,868,871,874,877,893,896,908,911,914,917,933,936,939,942,945,948,964,967]

result = [element for element in a if element not in b]
print(result)