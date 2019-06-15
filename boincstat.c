/*********************************************************************************
*** boincstat
*** Written by David Nunn
*** Shows brief details of current state of boinc files on system
*** Only works for setiathome workunits
*** 
*** To contact the author about bugs or anything else please email
*** davidzilch@hotmail.com
**********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <libxml/parser.h>
#ifndef NO_GETOPT_LONG
  #include <getopt.h>
#endif
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

#include "common.h"

#define VERSION 2.02

int totalslots ;

int checkCurDir(char *wdir)
{
	//checks for client_state to get the current working directory
	DIR *dirp;
	struct dirent *dp;	
	int errno;
	char cwd[90];

	dirp = opendir(".");
	while (dirp) 
	{
        	errno = 0;
         	if ((dp = readdir(dirp)) != NULL) 
		{
			struct stat sts;
	        	if (stat("client_state.xml",&sts)== 0)
			{
				//printf("client_state.xml exists\n");
				if (getcwd(cwd, 90) == NULL)
				{
					printf("Problem checking current working directory\n");
					return 2;
				}
				strcpy(wdir, cwd);
                 		closedir(dirp);
	                 	return 1;
			}
        	 } 
		else 
		{
	        	if (errno == 0) 
			{
                 		 closedir(dirp);
		                 return 0;
            		}
             		closedir(dirp);
		        return 0;
        	}
	}
	return 0;
}

void cmd_help()
{
	printf("boincstat - command line view of boinc files. Written by David Nunn\n\n");
	printf("Arguments:\n");
	printf("\t -d /your/boinc/dir - Tell program where your boinc directories are.\n\t\t\t As an alternative use the BOINCDIR environment variable\n\t\t\tor just put executable in your boinc directory.\n");
	printf("\t -p \t only show work unit in progress\n");
	printf("\t -s \t Shows summary info of all wu's on system\n");
	printf("\t -w \t Shows a continous display of inprogress workunits\n");
	printf("\t -v \t display version info\n");
	printf("\t -h \t display this help file\n");
	return;
}

void cmd_usage(char *argv)
{
	printf("Usage: %s -d /your/dir/to/boinc\n", argv);
	printf("%s -h for full help\n", argv);
	return;
} 


workunit* findWu(workunit* root,char *name, int type)
{
	//type = 0 for finding name
	//type = 1 for finding actname
	workunit* currwu;
	currwu = root;
	//printf("looking for %s\n", name);
	while(currwu->next!=NULL)
	{
		switch (type)
		{
		case 0:		
			if (!strcmp(name, currwu->name))
			{
				//found a match
				//printf("FOUND\n\n");
				return currwu;
			}  
			currwu=currwu->next;
			break;

		case 1:
			if (!strcmp(name, currwu->actname))
	                {
                	        //found a match
        	                //printf("FOUND\n\n");
                        	return currwu;
               		 }
			currwu=currwu->next;
                        break;
		}
	}

	// workunit not found
	printf("we seem to have a problem\n");
	return currwu;
}

workunit* newWu()
{
	workunit* newwu;
	newwu = malloc(sizeof(workunit));
	strcpy(newwu->name, "");
        strcpy(newwu->app_name , "");
	strcpy(newwu->app_version, "");
	newwu->final_cpu_time = 0.0;
	newwu->state = 0;
	newwu->slot = 0;
	newwu->cpu_used = 0;
	newwu->report_deadline = 0;
	newwu->start_time = 0;
	newwu->current_progress = 0;
	newwu->current_cpu = 0;
	newwu->next = NULL;
	strcpy(newwu->actname, "");
	return newwu;
}

void printWorkunits(workunit* root, int progress, int summary)
{
	workunit *currwu;
	currwu = root;
	int sumready = 0, sumfin = 0, sumprog = 0, hours = 0, mins = 0, secs = 0;
	float timeleft = 0;
	time_t temptime;
        struct tm* finishtime;
	char finishstring[35];
	struct timeval tv;
	struct timezone tz;
	if (!strcmp(currwu->name, ""))
		return;

	printf("\n");
	do
	{
		if (progress == 1)
		{
			if (currwu->state == 6)
			{
         			printf("Name                 : %s\n", currwu->name);
                                printf("Application          : %s\n", currwu->app_name);
        	                printf("App Version          : %s\n", currwu->app_version);
			}
        	        switch (currwu->state)
	        	{
                	case 0:
				sumready++;
               		        break;
			case 1:
				sumready++;
				break;
       		        case 2:
				sumready++;
              		        	break;
	       	        case 3:
				sumfin++;
              		        	break;
			case 4:
				sumfin++;
				break;
	      		case 5:
				sumfin++;
	       	                break;
			case 6:
				printf("State                : In Progress\n");
				sumprog++;
				break;
	       	        }
			if (currwu->state == 6)
			{
				printf("Progress             : %.2f%%\n", currwu->current_progress);
				timeleft = ((currwu->cpu_used / currwu->current_progress) *100) - currwu->cpu_used;
				hours = timeleft/3600;
				mins = ((timeleft-hours*3600)/60);
				secs = timeleft - hours*3600 - mins*60;
				//printf("Cpu = %f, timeleft = %f\n", currwu->cpu_used, timeleft);
				finishtime = localtime(&currwu->start_time);
                                strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
                                printf("Wu Started           : %s\n", finishstring);
                                printf("Est. Time Remaining  : %d:%02d:%02d\n", hours, mins, secs);
                                gettimeofday(&tv, &tz);
                                temptime = tv.tv_sec + timeleft;
                                finishtime = localtime(&temptime);
                                strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
                                printf("Est. Completion Time : %s\n", finishstring);
                                finishtime = localtime(&currwu->report_deadline);
                                strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
                                printf("Report Deadline      : %s\n\n", finishstring );
			}
		}
		else
		{
			printf("Name                 : %s\n", currwu->name);
			printf("Application          : %s\n", currwu->app_name);
			printf("App Version          : %s\n", currwu->app_version);
			switch (currwu->state)
			{
			case 0:
				printf("State                : Ready\n");
				sumready++;
				break;
			case 1:
				printf("State                : Ready\n");
				sumready++;
				break;
			case 2:
				printf("State                : Ready\n");
				sumready++;
				break;
			case 3:
				printf("State                : Finished\n");
				sumfin++;
				break;
			case 4:
				printf("State                : Finished\n");
				sumfin++;
				break;
			case 5:
				printf("State                : Finished\n");
				sumfin++;
				break;
			case 6:
				printf("State                : In Progress\n");
				sumprog++;
				timeleft = ((currwu->cpu_used / currwu->current_progress) *100) - currwu->cpu_used;
                                hours = timeleft/3600;
                                mins = ((timeleft-hours*3600)/60);
                                secs = timeleft - hours*3600 - mins*60;
				break;
			default:
				printf("State                : %d\n", currwu->state);
			}
			
			if(currwu->state == 6)
			{
				printf("Progress             : %.2f%%\n", currwu->current_progress);
				finishtime = localtime(&currwu->start_time);
                                strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
                                printf("Wu Started           : %s\n", finishstring);
                                printf("Est. Time Remaining  : %d:%02d:%02d\n", hours, mins, secs);
                                gettimeofday(&tv, &tz);
                                temptime = tv.tv_sec + timeleft;
                                finishtime = localtime(&temptime);
                                strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
                                printf("Est. Completion Time : %s\n", finishstring );
			}
			else
			{
				if (currwu->state >2)
				{
					printf("Progress             : %.2f\n", currwu->current_progress);
					printf("Final cpu            : %f\n", currwu->final_cpu_time);
				}
			}
			finishtime = localtime(&currwu->report_deadline);
                        strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
                        printf("Report Deadline      : %s\n\n", finishstring );
		}
		currwu = currwu->next;

	}	while(currwu->next!=NULL);

	if (summary)
	{
		printf("-------------------------------\n");
		printf("Summary info\n");
		printf("Total number of wu's Ready %d\n", sumready);
		printf("Total number of wu's Finished %d\n", sumfin);
		printf("Total number of wu's In Progress %d\n", sumprog);
		printf("-------------------------------\n");
	}
	return;
}

void printWorkunit(workunit* currwu)
{
	printf("Name       : %s\n", currwu->name);
	printf("Application: %s\n", currwu->app_name);
	printf("App Version: %s\n", currwu->app_version);
	printf("Report Deadline: %s\n", ctime(&currwu->report_deadline) );
	printf("Slot: %d\n", currwu->slot);
	return;
}

void deleteWorkunits(workunit* currwu)
{
	workunit *nextwu;
	while(currwu->next!=NULL)
	{
		nextwu = currwu->next;
		free(currwu);
		currwu = nextwu;
	}
	free(currwu);
	return;
}

void parseActive(xmlDocPtr doc, xmlNodePtr cur, char* docname, workunit* root)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	xmlNodePtr curAct;
	workunit* tempwu;
	workunit* currwu;
        tempwu = newWu();
	char slot[2];
	char tempname[90];
	struct stat sts;
	strcpy(tempname, docname);

	while (cur != NULL)
	{
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"active_task")))
                {
			curAct = cur->xmlChildrenNode;
			while (curAct != NULL)
			{
				if ((!xmlStrcmp(curAct->name, (const xmlChar *)"result_name")))
				{
					key = xmlNodeListGetString(doc,curAct->xmlChildrenNode, 1);
					strcpy(tempwu->name, key);				
		                        xmlFree(key);
				}
				if ((!xmlStrcmp(curAct->name, (const xmlChar *)"slot")))
				{
					key = xmlNodeListGetString(doc,curAct->xmlChildrenNode, 1);
                		        tempwu->slot = atoi(key);
                		        xmlFree(key);
				}
				if ((!xmlStrcmp(curAct->name, (const xmlChar *)"checkpoint_cpu_time")))
                                {
                                        key = xmlNodeListGetString(doc,curAct->xmlChildrenNode, 1);
					tempwu->cpu_used = atof(key);
					xmlFree(key);
				}

				curAct=curAct->next;
			}
			currwu = findWu(root, tempwu->name, 1);
			currwu->slot = tempwu->slot;
			currwu->cpu_used = tempwu->cpu_used;
			//printWorkunit(currwu);
		        slot[0] = (char)(currwu->slot + 48);
		        slot[1] = '\0';
		        strcat(docname, slot);
		        strcat(docname, "/state.sah");
			//printf("docname = %s\n", docname);
			totalslots++;
		        updateProgress(currwu, docname);
			strcpy(docname, tempname);
			strcat(docname, slot);
			strcat(docname, "/work_unit.sah");
                        if (stat(docname,&sts)== 0)
                        {
                                //printf("work_unit exists\n");
				//printf("Startime = %s", ctime(&(sts.st_mtime)));
				currwu->start_time = sts.st_mtime;
                        }
			strcpy(docname, tempname);
		}
		cur=cur->next;
	}

	free(tempwu);
	return;
}

void parseWorkunit(xmlDocPtr doc, xmlNodePtr cur, workunit* currwu)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
		{
			key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
			strcpy(currwu->name, key);
			//printf("Debug Name: %s\n", currwu->name);
			xmlFree(key);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"app_name")))
                {
                        key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
                        //printf("App Name: %s\n", key);
			strcpy(currwu->app_name,key);
                        xmlFree(key);
                }
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"version_num")))
                {
                        key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
                        //printf("App Version: %s\n", key);
			strcpy(currwu->app_version,key);
                        xmlFree(key);
                }
		cur=cur->next;
	}
	//printWorkunit(currwu);
	return;
}

void parseResult(xmlDocPtr doc, xmlNodePtr cur, workunit* root)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	workunit* currwu;
	workunit* tempwu;
	currwu = root;
	tempwu = newWu();

	while (cur != NULL) 
	{
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"wu_name"))) 
		{
		    key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
		    strcpy(tempwu->name, key);
		    xmlFree(key);
 	    	}
		if ((!xmlStrcmp(cur->name, (const xmlChar*)"report_deadline")))
		{
			key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
			int deadline = atoi(key);
			time_t deadtime;
			deadtime = deadline;
			char* strtime;
			strtime = ctime(&deadtime);
			tempwu->report_deadline=deadtime;
			xmlFree(key);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar*)"final_cpu_time")))
		{
			key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
			tempwu->final_cpu_time = atof(key);
			xmlFree(key);
		}
		
		if ((!xmlStrcmp(cur->name, (const xmlChar*)"state")))
		{
			key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
			tempwu->state= atoi(key);
			xmlFree(key);
		}
		if ((!xmlStrcmp(cur->name, (const xmlChar*)"name")))
		{
			key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
			strcpy(tempwu->actname, key);
			xmlFree(key);
		}
			
		//printWorkunit(currwu);		
		cur = cur->next;
	}

	// copy tempu into relevant details
	currwu = findWu(root, tempwu->name, 0);
	currwu->report_deadline = tempwu->report_deadline;
	currwu->final_cpu_time = tempwu->final_cpu_time;
	currwu->state = tempwu->state;
	if (currwu->state ==5 || currwu->state ==4)
	{
		currwu->current_progress = 100;
	}
	else
	{
		currwu->current_progress = tempwu->current_progress;
	}
	strcpy(currwu->actname, tempwu->actname);
	free(tempwu);

    return;
}


int main(int argc, char **argv) 
{

	char *docname; 
	char indir[90], indir1[90];
	float APP_VERSION = 2.02;
	
	int c;
	int opt_help = 0, opt_version = 0, opt_progress = 0, opt_summary = 0;
	int opt_window = 0;
	strcpy(indir,"");
	
	while(1)
	{
	#ifdef NO_GETOPT_LONG
 		c = getopt(argc, argv, "hpvswd:");
	#else
		static struct option long_options[] =
    		{
        		{"version", 0, 0, 'v'},
	        	{"help", 0, 0, 'h'},
			{"progress", 0, 0, 'p'},
		  	{"summary" , 0, 0, 's'},
			{"window", 0 , 0, 'w'}, 
			{"boincdir", 1, 0, 'd'},
        		{0, 0, 0, 0}
   		 };

  	  /* getopt_long stores the option index here. */

    		int option_index = 0;

		c = getopt_long (argc, argv, "hpvswd:",
        	     long_options, &option_index);
	#endif
		/* Detect the end of the options. */
   		if (c == -1)
     			break;
		 /*
	    	* Options with a straight flag, could use getoopt_long
    		* flag setting but this is more "obvious" and easier to
	    	* modify.
    			*/
   		switch (c)
     		{
     		case 'v':
       			opt_version = 1;
			break;
     		case 'h':
       			opt_help = 1;
       			break;
		case 'p':
			opt_progress = 1;
			break;
		case 's':
			opt_summary = 1;
			break;
		case 'w':
			opt_window = 1;
			break;
		case 'd':
			if (optarg == NULL)
			{ // error
				printf("option d specified but no directory\n");
				printf("optarg = %s\n",optarg);
				opt_help = 1;
				break;
			}
			strcpy(indir, optarg);
			strcpy(indir1, optarg);
			break;
     		case '?': // On error give help - getopt does a basic message.
       			opt_help = 1;
       			break;
     		default:
       			puts ("Option Processing Failed\n");
       			abort();
     		}
  	} // end of getopt_long style parsing

	if (opt_version == 1)
	{
		printf("boincstat version %.2f\n", APP_VERSION);
		return(0);
	}

	if (opt_help == 1)
	{
		cmd_help();
		return(0);
	}

	if (!strcmp(indir,"" ))
	{	// no dir specified so need to check env variable
		if (getenv("BOINCDIR") == NULL)
		{
			// so last resort check current dir
			int f = 0;
			f = checkCurDir(indir);
			strcpy(indir1, indir);
			if (f == 0)
			{
				printf("ERROR NO DIRECTORY SPECIFIED\n");
				cmd_usage(argv[0]);
                	        return(0);
			}
                }
		else
		{
			strcpy(indir, getenv("BOINCDIR"));
			strcpy(indir1, getenv("BOINCDIR"));
		}
	}
		

	xmlDocPtr doc;
	xmlNodePtr cur;
	int readagain = 0;
	strcat(indir, "/client_state.xml");
	strcat(indir1, "/slots/");
	
	while (readagain == 0)
	{
		totalslots = 0;
		docname = indir;
		doc = xmlParseFile(docname);
	
		if (doc == NULL ) 
		{
			fprintf(stderr,"Document not parsed successfully.\nCheck that you entered the correct directory for boinc\n");
			return(0);
		}

		cur = xmlDocGetRootElement(doc);
	
		if (cur == NULL) 
		{
			fprintf(stderr,"empty document\n");
			xmlFreeDoc(doc);
			return(0);
		}
	
		workunit *currwu;
		workunit *rootwu;
		rootwu = currwu = newWu();
	
		// Setup dirname for state file
		docname = indir1;

		cur = cur->xmlChildrenNode;
		while (cur != NULL) 
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar*)"workunit")))
			{
				parseWorkunit(doc, cur, currwu);
				currwu->next = newWu();
        	                currwu = currwu->next;
			}
			if ((!xmlStrcmp(cur->name, (const xmlChar*)"result")))
			{
				parseResult(doc, cur, rootwu);
			}
			if ((!xmlStrcmp(cur->name, (const xmlChar*)"active_task_set")))
			{
				parseActive(doc, cur, docname, rootwu);
			}
			cur = cur->next;
		}

		//Now look at state file for current progres
		if (opt_window == 1)
		{
			readagain = showWindow(rootwu, totalslots, docname);
		}
		else
		{
			printWorkunits(rootwu, opt_progress, opt_summary); 
			readagain = 1;
		}
		xmlCleanupParser();
		xmlFreeDoc(doc);
		deleteWorkunits(rootwu);
	} // end readagain process
	
	return (1);
}

