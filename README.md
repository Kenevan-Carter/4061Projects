# MapReduce

### Project Overview

Write a high-level overview of your MapReduce implementation here.
- - - - - - - - - - - - - - - - - - Main.c - - - - - - - - - - - - - - - - - -
    Main.c will act as the manager and organizer of the system.

    It is tasked with looping through each file and assigning it to a mapper.

    The program forks a given number of child processes who each individually call map.c in parallel. It then will wait for each child to finish.

    Once completed, it will split work for the reducers. Each reducer receives a [start] and [end] parameter (between 0-255) targetting the first byte of the IP address. This is important so that each child does not create a hash of the same address and count the same entry twice.

    Again, it waits for reducing to finish before it prints the contents of the combined table to the terminal.

- - - - - - - - - - - - - - - - - - Map.c - - - - - - - - - - - - - - - - - - -
    Each mapper will locally create a hash table to store key value pairs of (ip address, # of requests)

    Mappers receive files from command line arguments. Read each file line by line, extracting the ip address and hashing to the table. If the address exists already, increment the count and if not, set the value to 1.

    After completing, write the results to the intermediate file.

- - - - - - - - - - - - - - - - - - Reduce.c - - - - - - - - - - - - - - - - - -
    This program will take each intermediate hash table that the mapper child processes created and store it in one large master table. Each reducer will be in charge of a select range of addresses to look up and store. The master table is then filtered, keeping only the IPs whose first byte falls within the start,end range.

- - - - - - - - - - - - - - - - - - Table.c - - - - - - - - - - - - - - - - - - -
    This is the hash table that both map.c and reduce.c will create in order to easily store and look up ip addresses for total counts of request occurrences. The hash table uses separate chaining to handle collisions.

    When the mapper finishes counting IPs, it takes each bucket in the hash table and writes it directly to a file. When a reducer has to read that data, it reads one bucket at a time and reconstructs the data into a new hash table in its own memory. This is how the mapper and reducer communicate.


### Assumptions

Write any assumptions that we need to know about running your MapReduce implementation, or grading here.

### Work Split

Document the share of work that each group member did here. If you worked solo, please indicate so.

### AI Use

Document any AI use here. Refer to the writeup or the class syllabus if you are unsure of what is acceptable AI usage.
