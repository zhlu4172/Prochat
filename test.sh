#!/bin/bash

./server > global_log &

./tests/client1 > tests/_1_log
./tests/client1_1 > tests/_2_log
./tests/client1_2 > tests/_3_log
./tests/client1_2_1 > tests/_3_2_log
./tests/client2_1 > tests/_4_1_log
./tests/client2_2 > tests/_4_log


diff tests/_1_log tests/client1.out || echo "testcase failed"
diff tests/_2_log tests/client1_1.out || echo "test 2 failed"
diff tests/_3_log tests/client1_2.out || echo "test 3 failed"
diff tests/_3_2_log tests/client1_2_1.out || echo "test 3 failed"
diff tests/_4_log tests/client2_2.out || echo "test 4 failed"



