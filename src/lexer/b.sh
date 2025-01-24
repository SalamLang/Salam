clear

gcc -Wall -Wextra -Werror -g ../common/*.c *.c -o s

./s file test.txt

valgrind --leak-check=full --track-origins=yes ./s file test.txt
