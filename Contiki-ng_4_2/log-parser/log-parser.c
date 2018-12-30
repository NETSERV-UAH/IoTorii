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
#define CONDITION_MAX 5
#define HLMAC_STR_LEN_MAX 20
#define GLOBAL_STATISTICS 1  //To print a metric for all runs in a file

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
/* To find common ancestor between two Addresses*/
hlmacaddr_t get_lonest_common_prefix(hlmacaddr_t src, hlmacaddr_t dst)
{
  uint8_t min_len = (src.len < dst.len) ? src.len : dst.len;
  uint8_t i = 0;
  hlmacaddr_t commonPrexif = UNSPECIFIED_HLMAC_ADDRESS;

  while(i<min_len && (get_addr_index_value(src, i) == get_addr_index_value(dst, i)))
  {
    hlmac_add_new_id(&commonPrexif, i);
    i++;
  }
  return commonPrexif;
}
/*---------------------------------------------------------------------------*/
int num_hops_between_nodes(hlmacaddr_t src, hlmacaddr_t dst)
{
  hlmacaddr_t ancestor = get_lonest_common_prefix(src, dst);
  uint8_t ancestor_len = ancestor.len;
  if(ancestor.address != NULL){ //if ancestor == unspecified address, ancestor.address is NULL. for adding the first ID
    free(ancestor.address);
    ancestor.address = NULL;
    //return src.len - ancestor.len + dst.len - ancestor_len;
    return src.len + dst.len - 2 * ancestor_len;
  }
  /* For ancestor == unspecified address
   * when a node has not any HLMAC address, ancestor will be unspecified.
   */
  return -1;
}
/*---------------------------------------------------------------------------*/
void find_best_addr(unsigned int src_id, unsigned int dst_id, hlmacaddr_t *best_src, hlmacaddr_t *best_dst)
{
  if ((nodes[src_id] != NULL) && (nodes[dst_id] != NULL)){
    *best_src = nodes[src_id]->address; //We assiume the first src addr is the best one
    *best_dst = nodes[dst_id]->address; //We assiume the first dst addr is the best one
    int old_num_hop = num_hops_between_nodes(*best_src, *best_dst);
    if (old_num_hop != -1){
      for(hlmac_table_entery_t *src=nodes[src_id]; src!=NULL; src=src->next){
        for(hlmac_table_entery_t *dst=nodes[dst_id]; dst!=NULL; dst=dst->next){
          int new_num_hop = num_hops_between_nodes(src->address, dst->address);
          if((new_num_hop) && (old_num_hop > new_num_hop)){
            *best_src = src->address;
            *best_dst = dst->address;
            old_num_hop = new_num_hop;
          } //if
        } //for dst
      } //for src
    }else{ //if old_num_hop
      *best_src = *best_dst = UNSPECIFIED_HLMAC_ADDRESS; //There is not the best addresses
    }
  }else{
    *best_src = *best_dst = UNSPECIFIED_HLMAC_ADDRESS; //There is not the best addresses
  }
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
int exchange_addresses_in_node(unsigned int node_id, hlmacaddr_t old_hlmac, hlmacaddr_t new_hlmac)
{
    for(hlmac_table_entery_t *temp = nodes[node_id]; temp!=NULL; temp=temp->next){
      if (hlmac_cmp(temp->address, old_hlmac) == 0){
        //release memory for old entry
        free(temp->address.address);
        temp->address.address = NULL;
        //exchange pointers and variables
        temp->address.address = new_hlmac.address;
        temp->address.len = new_hlmac.len;
        //release memory
        free(old_hlmac.address);
        old_hlmac.address = NULL;

        return 1;

      }
    }

    return 0;

}

/*---------------------------------------------------------------------------*/
int log_file_parser(FILE *fp, char *destfile){

    char line[200];
    int i,common_check=0,check_condition[CONDITION_MAX]={0,0,0,0};

    for (int i=0; i<=NODE_NUMBERS_MAX; i++){
      nodes[i] = NULL;
    }
#if GLOBAL_STATISTICS == 1
    FILE *ConvergenceTime; //A file to save convergence time for all nodes
    if((ConvergenceTime=fopen("../ConvergenceTime.txt","a"))==NULL){
       fprintf(stderr,"\nCan't open destination file (../ConvergenceTime.txt)\n");
       return 0;

     }

     FILE *NumberOfMessages; //A file to save convergence time for all nodes
     if((NumberOfMessages=fopen("../NumberOfMessages.txt","a"))==NULL){
        fprintf(stderr,"\nCan't open destination file (../NumberOfMessages.txt)\n");
        return 0;

      }

      FILE *numberOfTableEntries; //A file to save convergence time for all nodes
      if((numberOfTableEntries=fopen("../numberOfTableEntries.txt","a"))==NULL){
         fprintf(stderr,"\nCan't open destination file (../numberOfTableEntries.txt)\n");
         return 0;

       }

       FILE *numberOfHops; //A file to save convergence time for all nodes
       if((numberOfHops=fopen("../numberOfHops.txt","a"))==NULL){
          fprintf(stderr,"\nCan't open destination file (../numberOfHops.txt)\n");
          return 0;

        }
#endif

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
                check_condition[0] = common_check && (strstr(line,"HLMAC address:")) && (strstr(line,"saved to HLMAC table"));
                check_condition[1] = common_check&&(strstr(line,"convergence_time_start"));
                check_condition[2] = common_check&&(strstr(line,"convergence_time_end"));
                check_condition[3] = common_check && (strstr(line,"node_id:")) && (strstr(line,"number_of_neighbours")); //strstr(line,"number_of_neighbours") : because "node_id:" exist in condition 1 and 2.
                check_condition[4] = common_check && (strstr(line,"new HLMAC address")) && (strstr(line,"is exchanged to old"));

                if(check_condition[0]){ //HLMAC addresses associated to each node

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

                if(check_condition[4]){ //exchange 2 address
                  unsigned int node_id;
                  char new_hlmac_str[HLMAC_STR_LEN_MAX];
                  char old_hlmac_str[HLMAC_STR_LEN_MAX];

                  sscanf(strstr(line,"ID:") + strlen("ID:"), "%u", &node_id);
                  sscanf(strstr(line,"new HLMAC address") + strlen("new HLMAC address"), "%s", new_hlmac_str);
                  sscanf(strstr(line,"is exchanged to old") + strlen("is exchanged to old"), "%s", old_hlmac_str);
                  printf("Node ID %u, new address %s is exchanget to address %s : ", node_id, new_hlmac_str, old_hlmac_str);
                  int done = exchange_addresses_in_node(node_id-1, hlmac_str_to_addr(old_hlmac_str), hlmac_str_to_addr(new_hlmac_str));
                  if(done){
                    printf("Done!\n");
                  }else{
                    printf("Not Done!\n");
                  }
                }

           }//END while

           //printf("convergence_time_start:\t%f(s)\n", convergence_time_start);
           //fputs("convergence_time_start\n",destfp);
           fprintf(destfp, "convergence_time_start\t%f\t(s)\n", convergence_time_start);

           //printf("convergence_time_end:\t%f(s)\n", convergence_time_end);
           fprintf(destfp, "convergence_time_end\t%f\t(s)\n", convergence_time_end);

           convergence_time = convergence_time_end - convergence_time_start;
           //printf("convergence_time:\t%f(s)\n", convergence_time);
           fprintf(destfp, "convergence_time\t%f\t(s)\n", convergence_time);
           #if GLOBAL_STATISTICS == 1
           fprintf(ConvergenceTime, "%f\n", convergence_time);
           #endif

           fputs("---------------------------------------------------------------\n",destfp);

           //print to std out
           //printf("node_id\tnumber_of_neighbours\tnumber_of_hlmac_addresses\tnumber_of_table_entries");
           //printf("\tnumber_of_hello_messages\tnumber_of_sethlmac_messages\tnumber_of_messages");
           //printf("\tsum_hop\taverage_sum_hop_for_node\n");

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
               //printf("%7u\t%20d\t%25d\t%23d\t%24d\t%27d\t%18d\t%7d\t%24f\n", i+1, number_of_neighbours[i], number_of_hlmac_addresses[i], number_of_table_entries[i], number_of_hello_messages[i], number_of_sethlmac_messages[i], number_of_messages[i], sum_hop[i], average_sum_hop_for_node);

               //print to file
               fprintf(destfp, "%7u\t%20d\t%25d\t%23d\t%24d\t%27d\t%18d\t%7d\t%24f\n", i+1, number_of_neighbours[i], number_of_hlmac_addresses[i], number_of_table_entries[i], number_of_hello_messages[i], number_of_sethlmac_messages[i], number_of_messages[i], sum_hop[i], average_sum_hop_for_node);
             }else{
               //print to std output
               //printf("%7u\t%20d\t%25d\t%23d\t%24d\t%27d\t%18d\t%7d\t           div_by_zerro!\n", i+1, number_of_neighbours[i], number_of_hlmac_addresses[i], number_of_table_entries[i], number_of_hello_messages[i], number_of_sethlmac_messages[i], number_of_messages[i], sum_hop[i]);

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

           fprintf(destfp, "Sum    \t%20f\t%25f\t%23f\t%24f\t%27f\t%18f\n", average_number_of_neighbours, average_number_of_hlmac_addresses, average_number_of_table_entries, average_number_of_hello_messages, average_number_of_sethlmac_messages, average_number_of_messages);

           //Average calculation
           average_sum_hop /= average_number_of_hlmac_addresses; //Now, average_number_of_hlmac_addresses includes the total number of the hlmac addresses
           average_number_of_neighbours /= node_id_max;
           average_number_of_hlmac_addresses /= node_id_max;
           average_number_of_hello_messages /= node_id_max;
           average_number_of_sethlmac_messages /= node_id_max;
           average_number_of_table_entries /= node_id_max;
           average_number_of_messages /= node_id_max;

           //print to std output
           //printf("Average\t%20f\t%25f\t%23f\t%24f\t%27f\t%18f\t%7f\n", average_number_of_neighbours, average_number_of_hlmac_addresses, average_number_of_table_entries, average_number_of_hello_messages, average_number_of_sethlmac_messages, average_number_of_messages, average_sum_hop);

           //print to file
           fprintf(destfp, "Average\t%20f\t%25f\t%23f\t%24f\t%27f\t%18f\t%7f\n", average_number_of_neighbours, average_number_of_hlmac_addresses, average_number_of_table_entries, average_number_of_hello_messages, average_number_of_sethlmac_messages, average_number_of_messages, average_sum_hop);

           #if GLOBAL_STATISTICS == 1
           fprintf(numberOfTableEntries, "%f\n", average_number_of_hlmac_addresses);
           fprintf(NumberOfMessages, "%f\n", average_number_of_sethlmac_messages);
           #endif



           fputs("---------------------------------------------------------------\n",destfp);

           //Print Addresses associated to each node
           //print to std out
           //printf("Addresses associated to each node:\nnode_id\tHLMAC addresses\n");

           //print to file
           fprintf(destfp, "Addresses which is associated to each node:\nnode_id\tHLMAC addresses\n");

           for (i=0; i<node_id_max; i++){
             //print to std out
             //printf("%-7d", i+1);
             //print to file
             fprintf(destfp, "%-7d", i+1);

             for(hlmac_table_entery_t *node_address_entry = nodes[i]; node_address_entry!=NULL; node_address_entry=node_address_entry->next){
               char *hlmac_addr_str;
               hlmac_addr_str = hlmac_addr_to_str(node_address_entry->address);
               //printf("\t%s", hlmac_addr_str);
               fprintf(destfp, "\t%s", hlmac_addr_str);
               free(hlmac_addr_str); //Because of malloc() in hlmac_addr_to_str()
             }
             //printf("\n");
             fprintf(destfp, "\n");
           }
           fputs("---------------------------------------------------------------\n",destfp);

           //Hop count calculation
           /*Create, set, and initailize a 2-dimentional array/matrix to hold
            *the metric values.
            */
           int **num_hops = (int **)malloc(sizeof(int *) * node_id_max);
           hlmacaddr_t best_src, best_dst;
           char *best_src_str, *best_dst_str;

           //Print header of table
           fprintf(destfp, "node_id");
           for(unsigned int i=0; i<node_id_max; i++){
             fprintf(destfp, "\t%u", i+1);
           }
           fprintf(destfp, "\n");

           for(unsigned int i=0; i<node_id_max; i++){
             fprintf(destfp, "%u\t", i+1); //The left collumn of the table

             num_hops[i] = (int *)malloc(sizeof(int) * node_id_max);
             for(unsigned int j=0; j<node_id_max; j++){
               if (i != j){
                 find_best_addr(i, j, &best_src, &best_dst);
                 num_hops[i][j] = num_hops_between_nodes(best_src, best_dst);
                 best_src_str = hlmac_addr_to_str(best_src);
                 best_dst_str = hlmac_addr_to_str(best_dst);
                 fprintf(destfp, "(src:%s => dst:%s, hops:%d)\t", best_src_str, best_dst_str, num_hops[i][j]);
                 free(best_src_str); //Because of malloc() in hlmac_addr_to_str()
                 best_src_str = NULL;
                 free(best_dst_str); //Because of malloc() in hlmac_addr_to_str()
                 best_dst_str = NULL;
                 /*
                  * If best_src.address be deleted (free(best_src.address)), the
                  * memory to which best_src.address refers (e.g. nodes[i]) is
                  * deleted.
                  *
                  *free(best_src.address);
                  *best_src.address = NULL;
                  *free(best_dst.address);
                  *best_dst.address = NULL;
                 */
               }else{
                 num_hops[i][j] = 0;
                 fprintf(destfp, "(%d)\t", num_hops[i][j]);
               }
             }
             fprintf(destfp, "\n");
           }

           fputs("---------------------------------------------------------------\n",destfp);

           //Average calculation of hop count
           float average_hop_count_mp2p = 0; //average hop count for Multi Point to Point traffic type
           float average_hop_count_p2mp = 0; //average hop count for Point to Multi Point traffic type
           float average_hop_count_p2p = 0;  //average hop count for Point to Point traffic type
           float average_hop_count_all = 0;  //average hop count for all traffic type

           int sum_hop_count_mp2p = 0;
           int sum_hop_count_p2mp = 0;
           int sum_hop_count_p2p = 0;
           int sum_hop_count_all = 0;

           for (unsigned int i=0; i<node_id_max; i++){
             for (unsigned int j=0; j<node_id_max; j++){
               if (num_hops[i][j] != -1){
                 /* All nodes,i, except the root node,i!=0, are src, and the root
                 * node,j==0, is dst.
                 */
                if ((i != 0) && (j == 0)){
                  average_hop_count_mp2p += num_hops[i][j];
                  sum_hop_count_mp2p ++;
                }
                /* The root node,i==0, is src, and all nodes,j, except the root
                 * node,j==0, is dst.
                 */
                if ((i == 0) && (j != 0)){
                  average_hop_count_p2mp += num_hops[i][j];
                  sum_hop_count_p2mp ++;
                }
                /* All nodes except the root node,i!=0 and j!=0, are src and dst,
                 * and src and dst are not the same, i!=j.
                 */
                if ((i != 0) && (j != 0 ) && (i != j)){
                  average_hop_count_p2p += num_hops[i][j];
                  sum_hop_count_p2p ++;
                }
                /* All nodes are src and dst,
                 * and src and dst are not the same, i!=j.
                 */
                if (i != j){
                  average_hop_count_all += num_hops[i][j];
                  sum_hop_count_all ++;
                }
               }
             }
           }

           //sum_hop_count_mp2p = node_id_max - 1
           average_hop_count_mp2p /= sum_hop_count_mp2p;
           //sum_hop_count_p2mp = node_id_max - 1
           average_hop_count_p2mp /= sum_hop_count_p2mp;
           //sum_hop_count_p2p = node_id_max * node_id_max - 3 * node_id_max + 2
           average_hop_count_p2p /= sum_hop_count_p2p;
           //sum_hop_count_all = node_id_max * node_id_max - node_id_max
           average_hop_count_all /= sum_hop_count_all;

           fprintf(destfp, "average_hop_count_mp2p\t%f\n", average_hop_count_mp2p);
           fprintf(destfp, "average_hop_count_p2mp\t%f\n", average_hop_count_p2mp);
           fprintf(destfp, "average_hop_count_p2p\t%f\n", average_hop_count_p2p);
           fprintf(destfp, "average_hop_count_all\t%f\n", average_hop_count_all);

           #if GLOBAL_STATISTICS == 1
           fprintf(numberOfHops, "%f\n", average_hop_count_all);
           #endif


           fclose(destfp);
           fclose(numberOfHops);
           fclose(ConvergenceTime);
           fclose(NumberOfMessages);
           fclose(numberOfTableEntries);

       }
       //Release memory
       //nodes array
       for (int i=0; i<NODE_NUMBERS_MAX; i++){
         if (nodes[i]){
           while(nodes[i]->next){
             hlmac_table_entery_t *temp = nodes[i];
             for (; temp->next->next; temp=temp->next);
             free(temp->next->address.address);
             temp->next->address.address = NULL;
             free(temp->next);
             temp->next = NULL;
           }
           free(nodes[i]->address.address);
           nodes[i]->address.address = NULL;
           free(nodes[i]);
           nodes[i] = NULL;
         }
       }
       return 1;
}
