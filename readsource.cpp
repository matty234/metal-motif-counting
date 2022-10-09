#include<iostream>
// for generating random reads
#include <random>
class Read {
    public:
        char * bases;
        // length of the read
        int length;
        void * encode() {

            // encode the bases into a bit array
            // 2 bits per base
            // 00 = A
            // 01 = C
            // 10 = G
            // 11 = T

            int num_bytes = strlen(bases) / 4;
            if (strlen(bases) % 4 != 0) {
                num_bytes++;
            }

            char * encoded = (char *) malloc(num_bytes);

            for (int i = 0; i < strlen(bases); i++) {
                char base = bases[i];
                int byte_index = i / 4;
                int bit_index = (i % 4) * 2;

                char encoded_base = 0;
                if (base == 'A') {
                    encoded_base = 0;
                } else if (base == 'C') {
                    encoded_base = 1;
                } else if (base == 'G') {
                    encoded_base = 2;
                } else if (base == 'T') {
                    encoded_base = 3;
                } else {
                    printf("Invalid base: %c\n", base);
                    exit(1);
                }

                encoded[byte_index] |= (encoded_base << bit_index);
            }

            return encoded;
        }
};


class ReadSource {

    public:
        void generate(int n, int read_length) {

            // generate n reads

            reads = (Read *) malloc(n * sizeof(Read));

            for (int i = 0; i < n; i++) {
                reads[i].bases = (char *) malloc(read_length);
                reads[i].length = read_length;
                for (int j = 0; j < reads[i].length; j++) {

                    // generate a random base
                    int base = rand() % 4;
                    if (base == 0) {
                        reads[i].bases[j] = 'A';
                    } else if (base == 1) {
                        reads[i].bases[j] = 'C';
                    } else if (base == 2) {
                        reads[i].bases[j] = 'G';
                    } else if (base == 3) {
                        reads[i].bases[j] = 'T';
                    } else {
                        printf("Invalid base: %d\n", base);
                        exit(1);
                    }
                }

            }

            num_reads = num_reads + n;
        }

        Read * getReads() {
            return reads;
        }

        Read * reads;
        int num_reads;

    private:


};