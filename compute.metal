#include <metal_stdlib>
using namespace metal;


// This will find all same-length motifs in 
kernel void find_motifs_in_encoded_sequnce(
    device const char *encoded_sequence,
    device const int *sequence_length,
    device const char *encoded_motifs,
    device const int *motif_length,
    device const int *motif_count,    
    device bool* result,
    uint index [[thread_position_in_grid]]
) {
    const device char * relevant_sequence = encoded_sequence + index * 1000;
    int relevant_sequence_length = sequence_length[index];

    int ml = as_type<int>(*motif_length);
    int mc = as_type<int>(*motif_count);
    int bool_size = sizeof(bool);
    
    for (int i = 0; i < relevant_sequence_length - ml; i++) {
        for (int j = 0; j < *motif_count; j++) {
            bool found = true;
            for (int k = 0; k < ml; k++) {
                if (relevant_sequence[k + i] != encoded_motifs[j * ml + k]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                result[(index * 1000 * bool_size * mc) + (i * bool_size * mc) + (j * bool_size)] = true;
            }
        }
    }
}
