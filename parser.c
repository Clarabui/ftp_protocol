/*
 * Parser.c
 * A simple Command Line Parser.
 * Author : Michael Roberts <mroberts@it.net.au>
 * Last Modification : 14/08/01
 */

#include "parser.h"

//#define DEBUG

/*
 * This function breakes the simple command token isolated in other functions
 * into a sequence of arguments. Each argument is bounded by white-spaces, and
 * there is no special character intepretation. The results are stored in the
 * argv array of the result command structure.
 * 
 * Arguments :
 *      cmd - the string to be processed.
 *      result - the comand struct to store the results in.
 *
 * Returns :
 *      None.
 *
 */
void
process_simple_cmd(char *cmd, command * result)
{
   char *dc;
   int lpc = 1;
#ifdef DEBUG
   fprintf(stderr,"process_simple_cmd\n");
#endif
   /*No Spaces Means No Arguments. */
   if (((dc = index(cmd, white_space[0])) == NULL
        && (dc = index(cmd, white_space[1])) == NULL)
       && (dc = index(cmd, (char) 9)) == NULL) {
      result->com_name = strdup(cmd);
      result->argv = realloc((void *) result->argv, sizeof(char *));
      result->argv[0] = strdup(cmd);
   }
   else {

      // nick modified this
      /*Pull out the Command Name (i.e First Token) */
      dc = strtok(cmd, white_space);
      //dc = cmd;
#ifdef DEBUG
      fprintf(stderr,"[%s][%s]\n",dc,cmd);
#endif
      result->com_name = strdup(dc);
      //result->com_name = strdup(cmd);
      // end nick
#ifdef DEBUG
      fprintf(stderr,"{1}\n");	
#endif
      result->argv = realloc((void *) result->argv, sizeof(char *));
#ifdef DEBUG
      fprintf(stderr,"{2}\n");	
#endif
      result->argv[0] = strdup(dc);


      /*Loop through the remaining tokens, writing them to the struct. */
      while ((dc = strtok(NULL, white_space)) != NULL) {
#ifdef DEBUG
	 fprintf(stderr,"[%s]\n",dc);
#endif
         result->argv = realloc((void *) result->argv, (lpc + 1) * sizeof(char *));
         result->argv[lpc] = strdup(dc);
         lpc++;
      }
   }
   /*Set the final array element NULL. */
   result->argv = realloc((void *) result->argv, (lpc + 1) * sizeof(char *));
   result->argv[lpc] = NULL;

   return;
}                       /*End of process_simple_cmd() */


/*
 * This function parses the commands isolated from the command line string in
 * other functions. It searches the string looking for input and output
 * redirection characters. The simple commands found are sent to
 * process_simple_comd(). The redirection information is stored in the result
 * command structure.
 *
 * Arguments :
 *      cmd - the command string to be processed.
 *      result - the command structure to store the results in.
 *
 * Returns :
 *      None.
 *
 */
void
process_cmd(char *cmd, command * result)
{
   char *pc, *mc;
   char *simple_cmd = NULL;

   /*If no redirection found, then only a simple command present. */
   if ((pc = index(cmd, '<')) == NULL) {
      if ((pc = index(cmd, '>')) == NULL) {
         process_simple_cmd(cmd, result);
         result->redirect_in = NULL;
         result->redirect_out = NULL;
      }
      else {            /*Output Redirection in place */
         pc = strtok(cmd, ">");
         simple_cmd = strdup(pc);

         pc = strtok(NULL, "\0");
         process_simple_cmd(simple_cmd, result);
         result->redirect_out = strdup(pc);
      }
   }
   else {               /*Input Redirection */
      pc = strtok(cmd, "<");
      simple_cmd = strdup(pc);
      pc = strtok(NULL, "\0");

      /*Output redirection may have been missed becuase input is checked
       * first.*/
      if ((mc = index(simple_cmd, '>')) != NULL)
         process_cmd(simple_cmd, result);
      if ((mc = index(pc, '>')) != NULL)
         process_cmd(pc, result);

      process_simple_cmd(simple_cmd, result);
      result->redirect_in = strdup(pc);
   }

   free(simple_cmd);
   return;
}                       /*End of process_cmd() */

/*
 * This function processes the command line. It isolates tokens seperated by
 * the '&' or the '|' character. The tokens are then passed on to be processed
 * by other functions. Once the first token has been isolated this function is
 * called recursivly to process the rest of the command line. Once the entire
 * command line has been processed an array of command structures is created
 * and returned.
 *
 * Arguments :
 *      cmd - the command line to be processed.
 *
 * Returns : 
 *      An array of pointers to command structures.
 *
 */
command **
process_cmd_line(char *cmd,int new)
{
   char *rc, *mc;
   char *rc_copy = NULL;
   static command **cmd_line;
   static int lc;

   // nick nelissen added this 23/9/01
   // ensures statics are null, when not recursively called 
   if(new==1){
	lc=0;
	cmd_line=NULL;
   }


   /*
    * Check for the existance of delimitors.
    * If no delimitors exist, we only have one command on the command line.
    * Otherwise process accordingly.
    */

   if ((rc = index(cmd, '&')) == NULL) {
      if ((rc = index(cmd, '|')) == NULL) {
         //cmd_line = realloc((void *) cmd_line, (lc + 1) * sizeof(command *));
	 cmd_line = realloc(cmd_line, (lc + 1) * sizeof(command *));
		 
	 if(cmd_line==NULL){
		exit(-1);
	 }

         cmd_line[lc] = malloc(sizeof(command));
         if(cmd_line[lc]==NULL){
		exit(-1);
	 }
	 // nick added this to NULL the new struct
         cmd_line[lc]->argv=NULL;
	 cmd_line[lc]->redirect_in=NULL;
	 cmd_line[lc]->redirect_out=NULL;
         cmd_line[lc]->com_name=NULL;
         cmd_line[lc]->pipe_to=0;
	 cmd_line[lc]->background=0;	 


         process_cmd(cmd, cmd_line[lc]);

         lc++;
      }
      else {            /*A '|' was found */
         rc = strtok(cmd, "|");
         rc = strtok(NULL, "");	/*Get the second token out */

         cmd_line = realloc((void *) cmd_line, (lc + 1) * sizeof(command *));
	 // nick changed this, same as nulling each element
         //cmd_line[lc] = malloc(sizeof(command));
         cmd_line[lc] = calloc(1,sizeof(command));

         process_cmd(cmd, cmd_line[lc]);
         cmd_line[lc]->pipe_to = lc + 1;
         lc++;
         if (rc != NULL)
            process_cmd_line(rc,0);	/*Process the Second Token */
      }
   }
   else {               /*A '&' was found */
      rc = strtok(cmd, "&");
      rc_copy = strdup(rc);	/*Make a copy of the first token */
      rc = strtok(NULL, "");	/*Get the second token out */
      if ((mc = index(rc_copy, '|')) != NULL) {
         process_cmd_line(rc_copy,0);
      }
      else {
         cmd_line = realloc((void *) cmd_line, (lc + 1) * sizeof(command *));
         // nick changed this
	 //cmd_line[lc] = malloc(sizeof(command));
	 cmd_line[lc] = calloc(1,sizeof(command));

         process_cmd(cmd, cmd_line[lc]);
         cmd_line[lc]->background = 1;
         lc++;
      }
      if (rc != NULL)
         process_cmd_line(rc,0);	/*Process the Second Token */

   }

   cmd_line = realloc((void *) cmd_line, (lc + 1) * sizeof(command *));
   cmd_line[lc] = NULL;

   free(rc_copy);

   return cmd_line;
}                       /*End of Process Cmd Line */

/*
 * This function cleans up some of the dynamicly allocated memory. Each array
 * element is visited, and the contained data is free'd before the entire
 * structure is free'd.
 *
 * Arguments :
 *      cmd - the array of pointers to command structures to be cleaned.
 *
 * Returns :
 *      None.
 *
 */
void
clean_up(command ** cmd)
{
   int lpc = 0;
   int ilpc = 0;

   while (cmd[lpc] != NULL) {
      ilpc = 0;
      if (cmd[lpc]->com_name != NULL)
         free(cmd[lpc]->com_name);	/*Free Com_Name */
      if (cmd[lpc]->argv != NULL) {
         while (cmd[lpc]->argv[ilpc] != NULL) {
            free(cmd[lpc]->argv[ilpc]);	/*Free each pointer in Argv */
            ilpc++;
         }
         free(cmd[lpc]->argv);	/*Free Argv Itself */
      }
      if (cmd[lpc]->redirect_in != NULL)
         free(cmd[lpc]->redirect_in);	/*Free Redirect - In */
      if (cmd[lpc]->redirect_out != NULL)
         free(cmd[lpc]->redirect_out);	/*Free Redirect - Out */
      free(cmd[lpc]);   /*Free the Command Structure */
      lpc++;
   }
   free(cmd);           /*Free the Array */
   cmd = NULL;
   return;
}                       /*End of clean_up() */


/*
 * This function dumps the contents of the structure to stdout.
 *
 * Arguments :
 *      c - the structure to be displayed.
 *      count - the array position of the structure.
 *
 * Returns :
 *      None.
 *
 */
void
dump_structure(command * c, int count)
{
   int lc = 0;

   printf("---- Command(%d) ----\n", count);
   printf("%s\n", c->com_name);
   if (c->argv != NULL) {
      while (c->argv[lc] != NULL) {
         printf("+-> argv[%d] = %s\n", lc, c->argv[lc]);
         lc++;
      }
   }
   printf("Background = %d\n", c->background);
   printf("Redirect Input = %s\n", c->redirect_in);
   printf("Redirect Output = %s\n", c->redirect_out);
   printf("Pipe to Command = %d\n\n", c->pipe_to);

   return;
}                       /*End of dump_structure() */

/*
 * This function dumps the contents of the structure to stdout in a human
 * readable format..
 *
 * Arguments :
 *      c - the structure to be displayed.
 *      count - the array position of the structure.
 *
 * Returns :
 *      None.
 *
 */
void
print_human_readable(command * c, int count)
{
   int lc = 1;

   printf("Program : %s\n", c->com_name);
   if (c->argv != NULL) {
      printf("Parameters : ");
      while (c->argv[lc] != NULL) {
         printf("%s ", c->argv[lc]);
         lc++;
      }
      printf("\n");
   }
   if (c->background == 1)
      printf("Execution in Background.\n");
   if (c->redirect_in != NULL)
      printf("Redirect Input from %s.\n", c->redirect_in);
   if (c->redirect_out != NULL)
      printf("Redirect Output to %s.\n", c->redirect_out);
   if (c->pipe_to != 0)
      printf("Pipe Output to Command# %d\n", c->pipe_to);
   printf("\n\n");

   return;
}                       /*End of print_human_readable() */


