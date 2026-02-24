# MapReduce

### Project Overview
Group Project by:
    Adam Rakowski ~ rakow026@umn.edu
    Kenevan Carter ~ cart0660@umn.edu

Context: We are given log files containing thousands of requests of entry and the process of extracting the ip addresses from each line in the file to find the number of requests per address takes a long time.

Problem: How can we extract these addresses quickly using parallel computation similar to that of mapReduce by Google. 



Write a high-level overview of your MapReduce implementation here.


- - - - - - - - - - - - - - - - - - Main.c - - - - - - - - - - - - - - - - - - 
    Main.c will act as the manager and organizer of the system.

    It is tasked with looping through each file and assigning it to a mapper. 

    The program forks a given number of child processes who each individually call map.c in parallel. It then will wait for each child to finish. 

    Once completed, it will split work for the reducers. Each reducer receives a [start] and [end] parameter (between 0-255) targetting the first byte of the IP address. This is important so that each child does not create a hash of the same address and count the same entry twice.
    
    Again, it waits for reducing to finish before it prints the contents of the combined table to the terminal.

- - - - - - - - - - - - - - - - - - Map.c - - - - - - - - - - - - - - - - - - - 
    Each mapper will locally create a hash table to store key value pairs of (ip address, # of requests)

    Read each file line by line, extracting the ip address and hashing to the table. If the address exists already, increment the count and if not, set the value to 1.

    After completing, write the results to the intermediate file.

- - - - - - - - - - - - - - - - - - Reduce.c - - - - - - - - - - - - - - - - - - 
    This program will take each intermediate hash table that the mapper child processes created and store it in one large on. Each reducer stores will be in charge of a select range of addresses to look up and store.
 
- - - - - - - - - - - - - - - - - - Table.c - - - - - - - - - - - - - - - - - - -
    This is the hash table that both map.c and reduce.c will create in order to easily store and look up ip addresses for total counts of request occurrences.



### Assumptions

Write any assumptions that we need to know about running your MapReduce implementation, or grading here.

### Work Split




### AI Use

Document any AI use here. Refer to the writeup or the class syllabus if you are unsure of what is acceptable AI usage.
