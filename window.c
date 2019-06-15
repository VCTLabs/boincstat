#include <curses.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "common.h"

static void finish(int sig);

void curseWorkunit(workunit *currwu)
{
	float timeleft = 0;
        time_t temptime;
        struct tm* finishtime;
        char finishstring[35];
        struct timeval tv;
        struct timezone tz;
        int hours = 0, mins = 0, secs = 0;

	attron(COLOR_PAIR(6));
        printw("Name                 : %s\n", currwu->name);
        printw("Application          : %s\n", currwu->app_name);
        printw("App Version          : %s\n", currwu->app_version);
        printw("Progress             : %.2f%%\n", currwu->current_progress);
        finishtime = localtime(&currwu->start_time);
        strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
        printw("Wu Started           : %s\n", finishstring);
        hours = currwu->cpu_used / 3600;
	mins = (currwu->cpu_used-hours*3600)/60;
        secs = currwu->cpu_used - hours*3600 - mins*60;
        printw("CPU Time             : %d:%02d:%02d\n", hours, mins, secs);
	timeleft = ((currwu->cpu_used / currwu->current_progress) *100) - currwu->cpu_used;
        hours = timeleft/3600;
        mins = ((timeleft-hours*3600)/60);
        secs = timeleft - hours*3600 - mins*60;
        //printf("Cpu = %f, timeleft = %f\n", currwu->cpu_used, timeleft);
        printw("Est. Time Remaining  : %d:%02d:%02d\n", hours, mins, secs);
        gettimeofday(&tv, &tz);
        temptime = tv.tv_sec + timeleft;
        finishtime = localtime(&temptime);
        strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
        printw("Est. Completion Time : %s\n", finishstring);
        finishtime = localtime(&currwu->report_deadline);
        strftime(finishstring, 34, "%a %b %d %H:%M:%S %Z %Y", finishtime);
        printw("Report Deadline      : %s\n\n", finishstring );
        attroff(COLOR_PAIR(6));
	
	return;
}

void updateProgress(workunit* currwu, char* docname)
{
        FILE *infile;
        //printf("opening %s\n", docname);
        if ((infile = fopen(docname, "r")) == NULL)
        {
                 fprintf(stderr, "Cannot open %s\n", docname);
                return;
        }
        char instring[80];
        int flag = 1;
        //printWorkunit(currwu);
        while (flag)
        {
                fgets(instring, 80, infile);
                //printf("just got %s\n", instring);
                if (strstr(instring, "prog")!=NULL)
                {
                        char progress[20];
                        strncpy(progress, instring+6, strlen(instring)-13);
                        strcpy(progress+6 , "\0");
                        currwu->state = 6;
                        currwu->current_progress = atof(progress)*100;
                        //printf("Current prog = %f\n", currwu->current_progress);
                        flag = 0;
                }
        }
        //printWorkunit(currwu);
        fclose(infile);
        return;
}

int showWindow(workunit* rootwu, int slots, char* docname)
{
	(void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

	(void) initscr();      /* initialize the curses library */
    	keypad(stdscr, TRUE);  /* enable keyboard mapping */
    	(void) nonl();         /* tell curses not to do NL->CR/NL on output */
    	(void) cbreak();       /* take input chars one at a time, no wait for \n */
    	(void) noecho();       /* don't echo input */
	halfdelay(200);

	workunit * currwu[slots];
	workunit * tempwu;
	tempwu  = rootwu;
	int found, x, c;
	int nostate = 0;

	for (x = 0; x < slots; x++)
	{
		found = 0;
		do 
		{
			if (tempwu->state == 0)
			{
				//probably couldn't open state.sah so just retry later
				move (10,0);
				printw("Problem displaying current workunit\n");
				printw("This may clear in a little while so either wait or quit and try again later.\n"); 
				nostate = 1;
				found = 1;
			}
			else
			{
				if (tempwu->state == 6)
				{
					currwu[x] = tempwu;
					found = 1;
				}
				else
				{
					tempwu = tempwu->next;
				}
			}

		} while (found == 0);
			
	}

	if (has_colors())
    	{
        	start_color();

        /*
         * Simple color assignment, often all we need.
         */
	        init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
        	init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
	        init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
        	init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
	        init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
        	init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	        init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
        	init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    	}

	int quit = 0, rc = 0, count = 0 ;
	char tempname[90];
	char slot[2];

    	for (;;)
	{
		count++;
		if (nostate == 1)
		{
			printw("\nPress q to exit");
			quit = 1;
			rc = 0;
		}
		else
		{
			move(0,0);
			attron(A_REVERSE);
			printw("The current status of in progress workunits\n\n");
			attroff(A_REVERSE);
			strcpy(tempname, docname);
			for (x=0; x<slots; x++)
			{
				slot[0] = (char)(x + 48);
	                        slot[1] = '\0';
                        	strcat(docname, slot);
                	        strcat(docname, "/state.sah");
        	                updateProgress(currwu[x], docname);
				curseWorkunit(currwu[x]);
				if (currwu[x]->current_progress == 1)
				{
					quit = 1;
					rc = 0;
				}
				strcpy(docname,tempname);
				printw("\nPress q to exit\n");
			}
		}

		refresh();
		
		c = getch();     /* refresh, accept single keystroke of input */
		switch (c)
                {
                case 'q':
                        quit = 1;
			rc = 1;
                        break;
		case 'r':
			quit = 1;
			rc   = 0;
			break;
                }

		if (count > 2)
		{
			quit = 1;
			rc = 0;
		}

		if (quit == 1)
                        break;
		

	}

	finish(0);               /* we're done */
	return rc;
}

static void finish(int sig)
{
    endwin();

    /* do your non-curses wrapup here */

    return;
}
