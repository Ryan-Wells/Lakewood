This is a simple program using pthreads to simulate a lake rental service. Renters show up and rent both life jackets and a vessel to go out on the lake. If there are not enough life jackets available, visitors will have to wait in line for some life jackets to be returned. Different vessels require a different number of life jackets:

	Kayaks require 1 life jacket
	Canoes require 2 life jackets
	Sailboats require 4 life jackets

The number of visitors is determined by the first argument provided and will arrive with a random frequency determined by the second argument provided when the program is executed (For example, ./Lakewood 20 10 r).  

The "r" in that example is used to control the randomness of the program. If left out, the program will run the same every time.



An example output is as follows:

./Lakewood 6 2 r
Group 0 requesting a sailboat with 4 lifevests.
Group 0 issued 4 lifevests, 6 remaining
Group 1 requesting a canoe with 2 lifevests.
Group 1 issued 2 lifevests, 4 remaining
Group 2 requesting a sailboat with 4 lifevests.
Group 2 issued 4 lifevests, 0 remaining
Group 3 requesting a kayak with 1 lifevests.
   Group 3 waiting in line for 1 lifevests
    Queue: [3]
Group 4 requesting a kayak with 1 lifevests.
   Group 4 waiting in line for 1 lifevests
    Queue: [3, 4]
Group 5 requesting a kayak with 1 lifevests.
   Group 5 waiting in line for 1 lifevests
    Queue: [3, 4, 5]
Group 0 returning 4 lifevests, now have 4
   Waiting group 3 may now proceed.
Group 3 issued 1 lifevests, 3 remaining
   Waiting group 4 may now proceed.
Group 4 issued 1 lifevests, 2 remaining
   Waiting group 5 may now proceed.
Group 5 issued 1 lifevests, 1 remaining
Group 1 returning 2 lifevests, now have 3
Group 2 returning 4 lifevests, now have 7
Group 4 returning 1 lifevests, now have 8
Group 5 returning 1 lifevests, now have 9
Group 3 returning 1 lifevests, now have 10



I made this project for Western Washington University's CSCI 347 as an assigned project.





