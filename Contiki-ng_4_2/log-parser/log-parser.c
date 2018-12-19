/*
 * Copyright (C) 2018 Elisa Rojas(1), Hedayat Hosseini(2), and David Carrascal(1);
 *                    (1) GIST, University of Alcala, Spain.
 *                    (2) CEIT, Amirkabir University of Technology (Tehran
 *                        Polytechnic), Iran.
 * To read and process the IoTorii log file generated in Contiki-ng
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hlmacaddr.h"

#define NODE_NUMBERS_MAX 500
#define CONDITION_MAX 4
#define HLMAC_STR_LEN_MAX 20

struct hlmac_table_entery{
  struct hlmac_table_entery *next;
  hlmacaddr_t address;
};

typedef struct hlmac_table_entery hlmac_table_entery_t;

/*
 * An array to hold a link list for each node.
 * Len of the array is equal to the number of nodes in a simulation.
 * The link list includes all HLMAC addresses of each node.
 * To find node_id_max, a log file must be parsed twice, so we used
 * a fixed len array to parse the log file once.
 */
//hlmac_table_entery_t **nodes;
//nodes = (hlmac_table_entery_t **) malloc (sizeof(hlmac_table_entery_t *) * node_id_max); //Must be placed after the node_id_max
hlmac_table_entery_t *nodes[NODE_NUMBERS_MAX];

int main(int argc, char *argv[])
{
    int log_file_parser(FILE *, char *);
    FILE *fp;
    char destfile[50] = "output_file_";

    printf("\nIoTorii log parser 2018\n\n");

    if (argc==1)
    {
        fprintf(stderr,"\nNo log file introduced\n");
    }
    else{
            if ((fp= fopen(*++argv,"r"))==NULL){
                fprintf(stderr,"\nCan't open %s\n",*argv);
                return 1;
            }else{
                strcat(destfile,*argv);
                if (log_file_parser(fp,destfile))
                    printf("\nNew file was generated successfully.\n");
                else
                    printf("\nNew file is not generated successfully! please try again.\n");
                fclose(fp);
            }

    }

    return 0;
}

/*---------------------------------------------------------------------------*/
void add_address_to_node(unsigned int node_id, hlmacaddr_t addr)
{
  hlmac_table_entery_t *node_addr;
  node_addr = (hlmac_table_entery_t *) malloc(sizeof(hlmac_table_entery_t));
  node_addr->address = addr;
  node_addr->next = NULL;

  hlmac_table_entery_t *temp = nodes[node_id];
  if (temp == NULL)
    nodes[node_id] = node_addr;
  else{
    for(; temp->next!=NULL; temp=temp->next);
    temp->next = node_addr;
  }
}

/*---------------------------------------------------------------------------*/
int log_file_parser(FILE *fp, char *destfile){

    char line[200];
    int i,common_check=0,check_condition[CONDITION_MAX]={0,0,0,0};

    for (int i=0; i<=NODE_NUMBERS_MAX; i++){
      nodes[i] = NULL;
    }

    FILE *destfp;
    if((destfp=fopen(destfile,"w"))==NULL){
       fprintf(stderr,"\nCan't open destination file (%s)\n",destfile);
       return 0;

       }else{
           strcat(line,"** This file was generated by IoTorii log parser **\n\n");
           fputs(line,destfp);

           //uint16_t node_id_max = 0;
           unsigned int node_id_max = 0;
           float convergence_time_end = 0;
           float convergence_time_start = 0;
           float convergence_time = 0;
           char field_name[50];
           int number_of_neighbours[NODE_NUMBERS_MAX], number_of_hlmac_addresses[NODE_NUMBERS_MAX];
           int number_of_hello_messages[NODE_NUMBERS_MAX], number_of_sethlmac_messages[NODE_NUMBERS_MAX];
           int number_of_table_entries[NODE_NUMBERS_MAX], number_of_messages[NODE_NUMBERS_MAX];
           int sum_hop[NODE_NUMBERS_MAX];
           float average_sum_hop_for_node = 0;

           //Variables to calculate average
           float average_number_of_neighbours = 0, average_number_of_hlmac_addresses = 0;
           float average_number_of_hello_messages = 0, average_number_of_sethlmac_messages = 0;
           float average_number_of_table_entries = 0, average_number_of_messages = 0;
           float average_sum_hop = 0;

           while (fgets(line,200,fp)){
                for(i=1;i<CONDITION_MAX;i++)
                    check_condition[i]=0;

                common_check=(strstr(line,"Periodic Statistics:")) && 1; // || 1 to avoid warning because type of common_check is int and the type of strstr() is char *.
                check_condition[0]=common_check && (strstr(line,"HLMAC address:")) && (strstr(line,"saved to HLMAC table"));
                check_condition[1]=common_check&&(strstr(line,"convergence_time_start"));
                check_condition[2]=common_check&&(strstr(line,"convergence_time_end"));
                check_condition[3]=common_check && (strstr(line,"node_id:")) && (strstr(line,"number_of_neighbours")); //strstr(line,"number_of_neighbours") : because "node_id:" exist in condition 1 and 2.

                if(check_condition[0]){ //HLMAC addresses
                  unsigned int node_id;
                  char hlmac_str[HLMAC_STR_LEN_MAX];
                  hlmacaddr_t temp;
                  sscanf(strstr(line,"ID:") + strlen("ID:"), "%d", &node_id);
                  sscanf(strstr(line,"HLMAC address:") + strlen("HLMAC address:"), "%s", hlmac_str);
                  add_address_to_node(node_id-1, hlmac_str_to_addr(hlmac_str));
                }

                if(check_condition[1]){ //convergence_time_start
                  int convergence_time_start_minute = 0;
                  float convergence_time_start_sec = 0;

                  //sscanf(strstr(line,"convergence_time_start:") + strlen("convergence_time_start:"), "%f", &convergence_time_start);
                  sscanf(line, "%2d:%f", &convergence_time_start_minute, &convergence_time_start_sec);
                  convergence_time_start = convergence_time_start_minute * 60 + convergence_time_start_sec;

                }

                if(check_condition[2]){ //convergence_time_end
                  float convergence_time_end_temp;
                  int convergence_time_end_temp_minute;
                  float convergence_time_end_temp_sec;
                  //sscanf(strstr(line,"convergence_time_end:") + strlen("convergence_time_end:"), "%f", &convergence_time_end_temp);
                  sscanf(line, "%2d:%f", &convergence_time_end_temp_minute, &convergence_time_end_temp_sec);
                  convergence_time_end_temp = convergence_time_end_temp_minute * 60 + convergence_time_end_temp_sec;
                  if (convergence_time_end_temp > convergence_time_end){
                    convergence_time_end = convergence_time_end_temp;
                  }

                }

                if(check_condition[3]){ //node_id
                  //uint16_t node_id;
                  unsigned int node_id;
                  sscanf(strstr(line,"node_id:") + strlen("node_id:"), "%d", &node_id);
                  sscanf(strstr(line,"number_of_neighbours:") + strlen("number_of_neighbours:"), "%d", &number_of_neighbours[node_id-1]);
                  sscanf(strstr(line,"number_of_hlmac_addresses:") + strlen("number_of_hlmac_addresses:"), "%d", &number_of_hlmac_addresses[node_id-1]);
                  sscanf(strstr(line,"number_of_hello_messages:") + strlen("number_of_hello_messages:"), "%d", &number_of_hello_messages[node_id-1]);
                  sscanf(strstr(line,"number_of_sethlmac_messages:") + strlen("number_of_sethlmac_messages:"), "%d", &number_of_sethlmac_messages[node_id-1]);
                  sscanf(strstr(line,"sum_hop:") + strlen("sum_hop:"), "%d", &sum_hop[node_id-1]);

                  number_of_table_entries[node_id-1] = number_of_neighbours[node_id-1] + number_of_hlmac_addresses[node_id-1];
                  number_of_messages[node_id-1] = number_of_hello_messages[node_id-1] + number_of_sethlmac_messages[node_id-1];
                  if (node_id > node_id_max)
                    node_id_max = node_id;
                }

           }//END while

           printf("convergence_time_start:\t%f\n", convergence_time_start);
           //fputs("convergence_time_start\n",destfp);
           fprintf(destfp, "convergence_time_start\t%f\n", convergence_time_start);

           printf("convergence_time_end:\t%f\n", convergence_time_end);
           fprintf(destfp, "convergence_time_end\t%f\n", convergence_time_end);

           convergence_time = convergence_time_end - convergence_time_start;
           printf("convergence_time:\t%f\n", convergence_time);
           fprintf(destfp, "convergence_time\t%f\n", convergence_time);
           fputs("---------------------------------------------------------------\n",destfp);

           //print to std out
           printf("node_id\tnumber_of_neighbours\tnumber_of_hlmac_addresses\tnumber_of_table_entries");
           printf("\tnumber_of_hello_messages\tnumber_of_sethlmac_messages\tnumber_of_messages");
           printf("\tsum_hop\taverage_sum_hop_for_node\n");

           //print to file
           fprintf(destfp, "node_id\tnumber_of_neighbours\tnumber_of_hlmac_addresses\tnumber_of_table_entries");
           fprintf(destfp, "\tnumber_of_hello_messages\tnumber_of_sethlmac_messages\tnumber_of_messages");
           fprintf(destfp, "\tsum_hop\taverage_sum_hop_for_node\n");
           //uint16_t i;
           unsigned int i;
           for (i=0; i<node_id_max; i++){
             //Calculating the average_sum_hop_for_node
             if (number_of_hlmac_addresses[i] != 0){
               average_sum_hop_for_node = sum_hop[i] / (float)number_of_hlmac_addresses[i];
               //print to std output
               printf("%7u\t%20d\t%25d\t%23d\t%24d\t%27d\t%18d\t%7d\t%24f\n", i+1, number_of_neighbours[i], number_of_hlmac_addresses[i], number_of_table_entries[i], number_of_hello_messages[i], number_of_sethlmac_messages[i], number_of_messages[i], sum_hop[i], average_sum_hop_for_node);

               //print to file
               fprintf(destfp, "%7u\t%20d\t%25d\t%23d\t%24d\t%27d\t%18d\t%7d\t%24f\n", i+1, number_of_neighbours[i], number_of_hlmac_addresses[i], number_of_table_entries[i], number_of_hello_messages[i], number_of_sethlmac_messages[i], number_of_messages[i], sum_hop[i], average_sum_hop_for_node);
             }else{
               //print to std output
               printf("%7u\t%20d\t%25d\t%23d\t%24d\t%27d\t%18d\t%7d\t           div_by_zerro!\n", i+1, number_of_neighbours[i], number_of_hlmac_addresses[i], number_of_table_entries[i], number_of_hello_messages[i], number_of_sethlmac_messages[i], number_of_messages[i], sum_hop[i]);

               //print to file
               fprintf(destfp, "%7u\t%20d\t%25d\t%23d\t%24d\t%27d\t%18d\t%7d\t           div_by_zerro!\n", i+1, number_of_neighbours[i], number_of_hlmac_addresses[i], number_of_table_entries[i], number_of_hello_messages[i], number_of_sethlmac_messages[i], number_of_messages[i], sum_hop[i]);
             }

             //Summation calculation
             average_number_of_neighbours += number_of_neighbours[i];
             average_number_of_hlmac_addresses += number_of_hlmac_addresses[i];
             average_number_of_hello_messages += number_of_hello_messages[i];
             average_number_of_sethlmac_messages += number_of_sethlmac_messages[i];
             average_number_of_table_entries += number_of_table_entries[i];
             average_number_of_messages += number_of_messages[i];
             average_sum_hop += sum_hop[i];
           }

           //Average calculation
           average_sum_hop /= average_number_of_hlmac_addresses; //Now, average_number_of_hlmac_addresses includes the total number of the hlmac addresses
           average_number_of_neighbours /= node_id_max;
           average_number_of_hlmac_addresses /= node_id_max;
           average_number_of_hello_messages /= node_id_max;
           average_number_of_sethlmac_messages /= node_id_max;
           average_number_of_table_entries /= node_id_max;
           average_number_of_messages /= node_id_max;

           //print to std output
           printf("Average\t%20f\t%25f\t%23f\t%24f\t%27f\t%18f\t%7f\n", average_number_of_neighbours, average_number_of_hlmac_addresses, average_number_of_table_entries, average_number_of_hello_messages, average_number_of_sethlmac_messages, average_number_of_messages, average_sum_hop);

           //print to file
           fprintf(destfp, "Average\t%20f\t%25f\t%23f\t%24f\t%27f\t%18f\t%7f\n", average_number_of_neighbours, average_number_of_hlmac_addresses, average_number_of_table_entries, average_number_of_hello_messages, average_number_of_sethlmac_messages, average_number_of_messages, average_sum_hop);


           fputs("---------------------------------------------------------------\n",destfp);

           //print to std out
           printf("Addresses associated to each node:\nnode_id\tHLMAC addresses\n");

           //print to file
           fprintf(destfp, "Addresses which is associated to each node:\nnode_id\tHLMAC addresses\n");

           for (i=0; i<node_id_max; i++){
             //print to std out
             printf("%-7d", i+1);
             //print to file
             fprintf(destfp, "%-7d", i+1);

             for(hlmac_table_entery_t *node_address_entry = nodes[i]; node_address_entry!=NULL; node_address_entry=node_address_entry->next){
               char *hlmac_addr_str;
               hlmac_addr_str = hlmac_addr_to_str(node_address_entry->address);
               printf("\t%s", hlmac_addr_str);
               fprintf(destfp, "\t%s", hlmac_addr_str);
               free(hlmac_addr_str); //Because of malloc() in hlmac_addr_to_str()
             }
             printf("\n");
             fprintf(destfp, "\n");
           }


           fclose(destfp);
       }
       return 1;
}