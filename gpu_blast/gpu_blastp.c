
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "gpu_blastp.h"
#include <algo/blast/core/gpu_cpu_common.h>


void Round2Multiple(int* const MaxLength)
{    
    int remainder = *MaxLength % WIDTH_MULTIPLE;
    
    if( 0 != remainder)
        *MaxLength += (WIDTH_MULTIPLE - remainder);
    
}

void MultipleCopyPV(PV_ARRAY_TYPE* h_RepeatedPV, PV_ARRAY_TYPE* h_pv, int pv_length, int num_blocksx)
{
    int offset = 0;
    for(int block = 0; block < num_blocksx; ++block)
    {
        memcpy(&h_RepeatedPV[offset], h_pv, pv_length*sizeof(PV_ARRAY_TYPE));
        offset += pv_length;
    }
}

void MultipleCopyMaxlength(int* h_Repeated_SequenceMaxlength_vector, const int* Sequence_Maxlength_vector, const int Group_number, const int stride, const int NumSequences)
{
    int offset = 0;
    for(int i = 0; i < NumSequences; ++i)
    {
        memcpy(&h_Repeated_SequenceMaxlength_vector[offset], Sequence_Maxlength_vector, Group_number*sizeof(int));
        offset += stride;
    }
}

void ReadSubstitutionMatrix(char *h_SubstitutionMatrix, char *h_RepeatedSubstitutionMatrix, int SubstitutionMatrix_length,  int num_blocksx)
{
    char Blosum62[28][28] = {
        {-128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128, -128}, //  (00)
        {-128,    4,   -2,    0,   -2,   -1,   -2,    0,   -2,   -1,   -1,   -1,   -1,   -2,   -1,   -1,   -1,    1,    0,    0,   -3,   -1,   -2,   -1,   -1,   -4,   -1,   -1}, //A (01) 
        {-128,   -2,    4,   -3,    4,    1,   -3,   -1,    0,   -3,    0,   -4,   -3,    4,   -2,    0,   -1,    0,   -1,   -3,   -4,   -1,   -3,    0,   -1,   -4,   -1,   -3}, //B (02)
        {-128,    0,   -3,    9,   -3,   -4,   -2,   -3,   -3,   -1,   -3,   -1,   -1,   -3,   -3,   -3,   -3,   -1,   -1,   -1,   -2,   -1,   -2,   -3,   -1,   -4,   -1,   -1}, //C (03)
        {-128,   -2,    4,   -3,    6,    2,   -3,   -1,   -1,   -3,   -1,   -4,   -3,    1,   -1,    0,   -2,    0,   -1,   -3,   -4,   -1,   -3,    1,   -1,   -4,   -1,   -3}, //D (04)
        {-128,   -1,    1,   -4,    2,    5,   -3,   -2,    0,   -3,    1,   -3,   -2,    0,   -1,    2,    0,    0,   -1,   -2,   -3,   -1,   -2,    4,   -1,   -4,   -1,   -3}, //E (05)
        {-128,   -2,   -3,   -2,   -3,   -3,    6,   -3,   -1,    0,   -3,    0,    0,   -3,   -4,   -3,   -3,   -2,   -2,   -1,    1,   -1,    3,   -3,   -1,   -4,   -1,    0}, //F (06)
        {-128,    0,   -1,   -3,   -1,   -2,   -3,    6,   -2,   -4,   -2,   -4,   -3,    0,   -2,   -2,   -2,    0,   -2,   -3,   -2,   -1,   -3,   -2,   -1,   -4,   -1,   -4}, //G (07)
        {-128,   -2,    0,   -3,   -1,    0,   -1,   -2,    8,   -3,   -1,   -3,   -2,    1,   -2,    0,    0,   -1,   -2,   -3,   -2,   -1,    2,    0,   -1,   -4,   -1,   -3}, //H (08)
        {-128,   -1,   -3,   -1,   -3,   -3,    0,   -4,   -3,    4,   -3,    2,    1,   -3,   -3,   -3,   -3,   -2,   -1,    3,   -3,   -1,   -1,   -3,   -1,   -4,   -1,    3}, //I (09)
        {-128,   -1,    0,   -3,   -1,    1,   -3,   -2,   -1,   -3,    5,   -2,   -1,    0,   -1,    1,    2,    0,   -1,   -2,   -3,   -1,   -2,    1,   -1,   -4,   -1,   -3}, //K (10)
        {-128,   -1,   -4,   -1,   -4,   -3,    0,   -4,   -3,    2,   -2,    4,    2,   -3,   -3,   -2,   -2,   -2,   -1,    1,   -2,   -1,   -1,   -3,   -1,   -4,   -1,    3}, //L (11)
        {-128,   -1,   -3,   -1,   -3,   -2,    0,   -3,   -2,    1,   -1,    2,    5,   -2,   -2,    0,   -1,   -1,   -1,    1,   -1,   -1,   -1,   -1,   -1,   -4,   -1,    2}, //M (12)
        {-128,   -2,    4,   -3,    1,    0,   -3,    0,    1,   -3,    0,   -3,   -2,    6,   -2,    0,    0,    1,    0,   -3,   -4,   -1,   -2,    0,   -1,   -4,   -1,   -3}, //N (13)
        {-128,   -1,   -2,   -3,   -1,   -1,   -4,   -2,   -2,   -3,   -1,   -3,   -2,   -2,    7,   -1,   -2,   -1,   -1,   -2,   -4,   -1,   -3,   -1,   -1,   -4,   -1,   -3}, //P (14)
        {-128,   -1,    0,   -3,    0,    2,   -3,   -2,    0,   -3,    1,   -2,    0,    0,   -1,    5,    1,    0,   -1,   -2,   -2,   -1,   -1,    4,   -1,   -4,   -1,   -2}, //Q (15)
        {-128,   -1,   -1,   -3,   -2,    0,   -3,   -2,    0,   -3,    2,   -2,   -1,    0,   -2,    1,    5,   -1,   -1,   -3,   -3,   -1,   -2,    0,   -1,   -4,   -1,   -2}, //R (16)
        {-128,    1,    0,   -1,    0,    0,   -2,    0,   -1,   -2,    0,   -2,   -1,    1,   -1,    0,   -1,    4,    1,   -2,   -3,   -1,   -2,    0,   -1,   -4,   -1,   -2}, //S (17)
        {-128,    0,   -1,   -1,   -1,   -1,   -2,   -2,   -2,   -1,   -1,   -1,   -1,    0,   -1,   -1,   -1,    1,    5,    0,   -2,   -1,   -2,   -1,   -1,   -4,   -1,   -1}, //T (18)
        {-128,    0,   -3,   -1,   -3,   -2,   -1,   -3,   -3,    3,   -2,    1,    1,   -3,   -2,   -2,   -3,   -2,    0,    4,   -3,   -1,   -1,   -2,   -1,   -4,   -1,    2}, //V (19)
        {-128,   -3,   -4,   -2,   -4,   -3,    1,   -2,   -2,   -3,   -3,   -2,   -1,   -4,   -4,   -2,   -3,   -3,   -2,   -3,   11,   -1,    2,   -2,   -1,   -4,   -1,   -2}, //W (20)
        {-128,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -4,   -1,   -1}, //X (21)
        {-128,   -2,   -3,   -2,   -3,   -2,    3,   -3,    2,   -1,   -2,   -1,   -1,   -2,   -3,   -1,   -2,   -2,   -2,   -1,    2,   -1,    7,   -2,   -1,   -4,   -1,   -1}, //Y (22)
        {-128,   -1,    0,   -3,    1,    4,   -3,   -2,    0,   -3,    1,   -3,   -1,    0,   -1,    4,    0,    0,   -1,   -2,   -2,   -1,   -2,    4,   -1,   -4,   -1,   -3}, //Z (23)
        {-128,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -4,   -1,   -1}, //  (24)
        {-128,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,    1,   -4,   -4}, //  (25)
        {-128,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -4,   -1,   -1}, //  (26)
        {-128,   -1,   -3,   -1,   -3,   -3,    0,   -4,   -3,    3,   -3,    3,    2,   -3,   -3,   -2,   -2,   -2,   -1,    2,   -2,   -1,   -1,   -3,   -1,   -4,   -1,    3}  //  (27)
    };
    
    memcpy(h_SubstitutionMatrix, Blosum62,SubstitutionMatrix_length*sizeof(char) );
    
    int offset = 0;
    for(int block = 0; block < num_blocksx; ++block)
    {
        memcpy(&h_RepeatedSubstitutionMatrix[offset], h_SubstitutionMatrix, SubstitutionMatrix_length*sizeof(char));
        offset += SubstitutionMatrix_length*sizeof(char);
    }

}



