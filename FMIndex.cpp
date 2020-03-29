#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <stdlib.h>
#include<sys/time.h>

using namespace std;

//-----------------------DO NOT CHANGE NAMES, ONLY MODIFY VALUES--------------------------------------------

//Final Values that will be compared for correctness
//You may change the function prototypes and definitions, but you need to present final results in these arrays
//-----------------------------Structures for correctness check-------------------
int **SA_Final_student;
int **L_counts_student;
char *L_student;
int F_counts_student[]={0,0,0,0};
//--------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------


//-----------------------DO NOT CHANGE--------------------------------------------

int read_count = 0;
int read_length = 0;

int **SA_Final;
int **L_counts;
char *L;
int F_counts[]={0,0,0,0};


//Read file to get reads
char** inputReads(char *file_path, int *read_count, int *length){
    FILE *read_file = fopen(file_path, "r");
    int ch, lines=0;
    char **reads;
    do                                                                                                 
    {                                                                                                  
        ch = fgetc(read_file);                                                                            
        if (ch == '\n')                                                                                
            lines++;                                                                                   
    } while (ch != EOF);
    rewind(read_file);
    reads=(char**)malloc(lines*sizeof(char*));
    *read_count = lines;
    int i = 0;                                                                                         
    size_t len = 0;                                                                                    
    for(i = 0; i < lines; i++)                                                                         
    {
        reads[i] = NULL;
        len = 0;                                                                                
        getline(&reads[i], &len, read_file);
    }                                                                                                  
    fclose(read_file);
    int j=0;
    while(reads[0][j]!='\n')
        j++;
    *length = j+1;
    for(i=0;i<lines;i++)
        reads[i][j]='$';
    return reads;
}

//Check correctness of values
int checker(){
    int correct = 1;
    for(int i=0; i<read_count*read_length;i++){
        if(L_student[i]!=L[i])
            correct = 0;
        for(int j=0;j<2;j++){
            if(SA_Final_student[i][j]!=SA_Final[i][j])
                correct = 0;
        }
        for(int j=0;j<4;j++){
            if(L_counts_student[i][j]!=L_counts[i][j])
                correct = 0;
        }
    }
    for(int i=0;i<4;i++){
        if(F_counts_student[i]!=F_counts[i])
            correct = 0;
    }
    return correct;
}

//Rotate read by 1 character
void rotateRead(char *read, char *rotatedRead, int length){
    for(int i=0;i<length-1;i++)
        rotatedRead[i]=read[i+1];
    rotatedRead[length-1]=read[0];
}

//Generate Sufixes and their SA's for a read
char** generateSuffixes(char *read, int length, int read_id){
    char **suffixes=(char**)malloc(length*sizeof(char*));
    suffixes[0]=(char*)malloc(length*sizeof(char));
    for(int j=0;j<length;j++)
        suffixes[0][j]=read[j];
    for(int i=1;i<length;i++){
        suffixes[i]=(char*)malloc(length*sizeof(char));
        rotateRead(suffixes[i-1], suffixes[i], length);
    }
    return suffixes;
}

//Comparator for Suffixes
int compSuffixes(char *suffix1, char *suffix2, int length){
    int ret = 0;
    for(int i=0;i<length;i++){
        if(suffix1[i]>suffix2[i])
            return 1;
        else if(suffix1[i]<suffix2[i])
            return -1;
    }
    return ret;
}


//Calculates the final FM-Index
int** makeFMIndex(char ***suffixes, int read_count, int read_length, int F_count[], char *L){
    int i, j;

    SA_Final=(int**)malloc(read_count*read_length*sizeof(int*));
    for(i=0;i<read_count*read_length;i++)
        SA_Final[i]=(int*)malloc(2*sizeof(int));

    //Temporary storage for collecting together all suffixes
    char **temp_suffixes=(char**)malloc(read_count*read_length*sizeof(char*));

    //Initalization of temporary storage
    for(i=0;i<read_count;i++){
        for(j=0;j<read_length;j++){
            temp_suffixes[i*read_length+j]=(char*)malloc(read_length*sizeof(char));
            memcpy(&temp_suffixes[i*read_length+j], &suffixes[i][j],read_length*sizeof(char));
            SA_Final[i*read_length+j][0]=j;
            SA_Final[i*read_length+j][1]=i;
        }
    }
    
    char *temp=(char*)malloc(read_length*sizeof(char));
    
    int **L_count=(int**)malloc(read_length*read_count*sizeof(int*));
    for(i=0;i<read_length*read_count;i++){
        L_count[i]=(int*)malloc(4*sizeof(int));
        for(j=0;j<4;j++){
            L_count[i][j]=0;
        }
    }

    //Focus on improving this for evaluation purpose
    //Sorting of suffixes
    for(i=0;i<read_count*read_length-1;i++){
        for(j=0;j<read_count*read_length-i-1;j++){
            if(compSuffixes(temp_suffixes[j], temp_suffixes[j+1], read_length)>0){
                memcpy(temp, temp_suffixes[j], read_length*sizeof(char));
                memcpy(temp_suffixes[j], temp_suffixes[j+1], read_length*sizeof(char));
                memcpy(temp_suffixes[j+1], temp, read_length*sizeof(char));
                int temp_int = SA_Final[j][0];
                SA_Final[j][0]=SA_Final[j+1][0];
                SA_Final[j+1][0]=temp_int;
                temp_int = SA_Final[j][1];
                SA_Final[j][1]=SA_Final[j+1][1];
                SA_Final[j+1][1]=temp_int;
            }
        }
    }

    free(temp);
    char this_F = '$';
    j=0;
    
    //Calculation of F_count's
    for(i=0;i<read_count*read_length;i++){
        int count=0;
        while(temp_suffixes[i][0]==this_F){
            count++;i++;
        }
        F_count[j++]=j==0?count:count+1;
        this_F = temp_suffixes[i][0];
        if(temp_suffixes[i][0]=='T')
            break;
    }
    
    //Calculation of L's and L_count's
    for(i=0;i<read_count*read_length;i++){
        char ch = temp_suffixes[i][read_length-1];
        L[i]=ch;
        if(i>0){
            for(int k=0;k<4;k++)
                L_count[i][k]=L_count[i-1][k];
        }
        if(ch=='A')
            L_count[i][0]++;
        else if(ch=='C')
            L_count[i][1]++;
        else if(ch=='G')
            L_count[i][2]++;
        else if(ch=='T')
            L_count[i][3]++;
    }

    return L_count;
}

//-----------------------DO NOT CHANGE--------------------------------------------

int main(int argc, char *argv[]){

    char * read_data_file= "COsmall.txt";   // input DATA

    char **reads = inputReads(read_data_file, &read_count, &read_length);//Input reads from file

    char ***suffixes=(char***)malloc(read_count*sizeof(char**));//Storage for read-wise suffixes
  

    //-----------------------------Structures for correctness check----------------------------------------------
    L=(char*)malloc(read_count*read_length*sizeof(char*));//Final storage for last column of sorted suffixes
    //-----------------------------Structures for correctness check----------------------------------------------
    

    //-----------Default implementation----------------
    //-----------Time capture start--------------------
    struct timeval  TimeValue_Start;
    struct timeval  TimeValue_Final;
    struct timezone TimeZone_Start;
    struct timezone TimeZone_Final;
    long time_start, time_end;
    double time_overhead_default, time_overhead_student;

    gettimeofday(&TimeValue_Start, &TimeZone_Start);


    //Generate read-wise suffixes
    for(int i=0;i<read_count;i++){
        suffixes[i]=generateSuffixes(reads[i], read_length, i);
    }
   

    //Calculate finl FM-Index
    L_counts = makeFMIndex(suffixes, read_count, read_length, F_counts, L);
    
    gettimeofday(&TimeValue_Final, &TimeZone_Final);
    time_start = TimeValue_Start.tv_sec * 1000000 + TimeValue_Start.tv_usec;
    time_end = TimeValue_Final.tv_sec * 1000000 + TimeValue_Final.tv_usec;
    time_overhead_default = (time_end - time_start)/1000000.0;
    //------------Time capture end----------------------
    //--------------------------------------------------


    //-----------Your implementations------------------
    gettimeofday(&TimeValue_Final, &TimeZone_Final);
    time_start = TimeValue_Start.tv_sec * 1000000 + TimeValue_Start.tv_usec;
    //-----------Call your functions here--------------------

    //-----------Call your functions here--------------------
    time_end = TimeValue_Final.tv_sec * 1000000 + TimeValue_Final.tv_usec;
    time_overhead_student = (time_end - time_start)/1000000.0;
    //--------------------------------------------------

    //----------------For debug purpose only-----------------
    //for(int i=0;i<read_count*read_length;i++)        
    //    cout<<L[i]<<"\t"<<SA_Final[i][0]<<","<<SA_Final[i][1]<<"\t"<<L_counts[i][0]<<","<<L_counts[i][1]<<","<<L_counts[i][2]<<","<<L_counts[i][3]<<endl;
    //--------------------------------------------------

    //---------------Correction check and speedup calculation----------------------
    float speedup=0.0;
    //if(checker()==1)
    //    speedup = time_overhead_default/time_overhead_student;
    cout<<"Speedup="<<speedup<<endl;
    //-----------------------------------------------------------------------------
    return 0;
}
