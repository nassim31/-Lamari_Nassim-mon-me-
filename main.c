

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
# include <pwd.h>
# include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ARGS_SIZE 150
#define DELIM " \t\r\n\a''$'\373'"
#define LINE_SIZE 2000


int pid=-1;
int pid1=-1;
int pid2=-1;
int iswriten=0;
int isredirect=0;
int writentype=99;
int isreading=0;
int pipeposition[2]={0,0};
int ispipe=0;
/*
int toprocess=0;
int argsposition[2][2];
	*/
	
char filewrite[80];//remet les a zero apres
char fileread[80];	
	
int cd(char **args);
int ls(char **args);
int help(char **args);
int myexit(char **args);
int clear(char **args);
int rmDir(char **args);
int pwd(char **args);
void prompt();
void handler_ctrlc(int sig);

char **line_order(char* line);

void handler_ctrlc(int sig){
    if(pid!=-1){
        printf("\n sigcall \n");
        kill(pid, SIGINT);

    }
    prompt();
}

char *builtin_str[] = {"icd", "ils", "ihelp", "iexit", "iclear", "irmdir", "ipwd"};


int (*builtin_func[]) (char **) = {
        &cd,
        &ls,
        &help,
        &myexit,
        &clear,
        &rmDir,
        &pwd

};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}


int cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, " expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("chdir");
        }
    }
    return 1;
}
int ls(char **args){

    struct dirent *lecture;
    DIR *rep;
    rep = opendir("." );
    while ((lecture = readdir(rep))) {
        printf("%s\t", lecture->d_name);
    }
    closedir(rep);
    return 1;
}



int help(char **args)
{
    int i;
    printf(" \n");
    printf("entrer votre commande.\n");
    printf("c est mes builtin: \n");

    for (i = 0; i < num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf(".\n");
    return 1;
}


int myexit(char **args)
{
    return 0;
}


int clear(char **args){
    printf("\033[2J\033[1;1H");
    return 1;
}

int rmDir(char **args)
{
    if (args[1] == NULL)
    {
        printf("\n tihs function is used with a valid path\n");
        return 1;

    } else{
        if(rmdir(args[1])!=0) {
            perror("rmdir ");
        }
    }
    return 1;
}
int pwd (char**args){
    char pwd[500];
    getcwd(pwd, 500);
    printf("\033[0;32m");
    printf("%s\n", pwd);
    printf("\033[0m");
    return 1;
}

void prompt(){

    char hostname[50];
    gethostname(hostname,50);
    uid_t uid;
    char cwdir[500];
    uid = getuid();
    struct passwd *user;
    user = getpwuid(uid);
    printf("\033[1;31m");
    printf("\n%s",user->pw_name);
    printf("\033[0;34m");
    printf("@%s:",hostname);
    printf("\033[1;36m");
    getcwd(cwdir, 500);
    printf("%s $ ",cwdir);
    printf("\033[0m");
}




int launch(char **args)
{
if(!ispipe){
		int status;
		int forkval;

		pid = fork();
		if (pid == 0) {//enfant
			int in ;
			int out;
			int errorout;

			if(iswriten)
			{ 
				if(writentype==1 || writentype==2 || writentype==3){
					if (writentype==1)
					{
					 out = open(filewrite, O_CREAT | O_WRONLY | O_APPEND , 0600);//writen type 1 append	>>
					}else if (writentype==2)
					{
					out = open(filewrite, O_CREAT | O_WRONLY | O_TRUNC , 0600);//writen type 2 trunk >
					}else{
						out = open(filewrite, O_CREAT | O_WRONLY | O_TRUNC , 0600);
						errorout=open(filewrite, O_CREAT | O_WRONLY | O_TRUNC , 0600);//STDERR_FILENO
						dup2(errorout, STDERR_FILENO);
						close(errorout);
						}
				
				/* Replace stdout with the file */
				dup2(out, STDOUT_FILENO);
				close(out);			
				
				}}
				
				if(isreading){
					
					in = open(fileread, O_RDONLY);//reading file <
					dup2(in, STDIN_FILENO);
					close(in);
					}	
			
			 signal(SIGINT, NULL);
			if (execvp(args[0], args) == -1) {
			   
				
				perror("execvp");
			}
			exit(EXIT_FAILURE);
		} else if (pid < 0) {

			perror("fork");
		} else {//PAPA
			
			do {
				
				waitpid(pid, &status, WUNTRACED);
				pid=-1;
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		
		
		iswriten=0;// remetre rédirection a zéro
		writentype=99; 
		isreading=0; 
		} 
		           
		return 1;
	}else if(ispipe){ 
		
	int status1;	
	int status2;	
		
	int pp[2];
	pipe(pp);

	pid1 = fork();

	if( pid1 == 0) {
		/* Replace stdout with the write end of the pipe */
		if(isreading){
					
		int in = open(fileread, O_RDONLY);//reading file <
		dup2(in, STDIN_FILENO);
		close(in);
		}
		
		
		
		dup2(pp[1], STDOUT_FILENO);
		/* Close read end of the pipe */
		close(pp[0]);
		close(pp[1]);
		/* Run command */


        signal(SIGINT, NULL);
		execvp( args[0], args);
	}
	else {
		pid2 = fork();
	
		if(pid2 == 0) {
			/* Replace stdin with the read end of the pipe */
			dup2(pp[0], STDIN_FILENO);
			
			if(iswriten)
			{ 
				int out;
				int errorout;
				if(writentype==1 || writentype==2 || writentype==3)
				{
					if (writentype==1)
					{
					 out = open(filewrite, O_CREAT | O_WRONLY | O_APPEND , 0600);//writen type 1 append	>>
					}else if (writentype==2)
					{
					out = open(filewrite, O_CREAT | O_WRONLY | O_TRUNC , 0600);//writen type 2 trunk >
					}else
					{
						out = open(filewrite, O_CREAT | O_WRONLY | O_TRUNC , 0600);
						errorout=open(filewrite, O_CREAT | O_WRONLY | O_TRUNC , 0600);//STDERR_FILENO
						dup2(errorout, STDERR_FILENO);
						close(errorout);
					}						
				}
				
				/* Replace stdout with the file */
				dup2(out, STDOUT_FILENO);
				close(out);			
				
				}
			
			
			close(pp[1]);
			close(pp[0]);



            signal(SIGINT, NULL);
			execvp( args[pipeposition[1]], args);
		}else {//PAPPA
			/* Close both end of the pipe */
			close(pp[0]);
			close(pp[1]);
			/* wait for two child */
			do {
				
				waitpid(pid1, &status1, WUNTRACED);
				pid1=-1;
			} while (!WIFEXITED(status1) && !WIFSIGNALED(status1));
			do {
				
				waitpid(pid2, &status2, WUNTRACED);
				pid2=-1;
			} while (!WIFEXITED(status2) && !WIFSIGNALED(status2));
			iswriten=0;
			writentype=99;
			isreading=0;
			pipeposition[0]=0;
			pipeposition[1]=0;
			ispipe=0;
		}
	}
		
return 1;
		
		}
    
 } 


int execute(char **args)
{
    int i;

    if (args[0] == NULL) {
    
        return 1;
    }

    for (i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return launch(args);
}


char **line_order(char* line)
{

    if (line)
    {
        free (line);
        line= (char *)NULL;
    }
    size_t linesize = LINE_SIZE;
    line =(char*) malloc(sizeof(char) * linesize);

    if (!line ) {
        fprintf(stderr, " allocation error\n");
        exit(EXIT_FAILURE);
    }
    int c;
    int watchline=0;

    while( (c=fgetc(stdin)) !='\n')
    {

		
        if(EOF==c)
        {
            printf("\n see you next time\n");
            free(line);
            exit(0);
        }
        
        if(c=='~')
        {//géré absolute path
            uid_t uid;
            uid = getuid();
            struct passwd *user;
            user = getpwuid(uid);
            printf("\n%s\n",user->pw_dir);
         int decalage=strlen(user->pw_dir);
             char tempo;
             int icount=0;
			while(decalage!=icount){
				line[watchline+icount]=user->pw_dir[icount];
				icount++;
				    }
         watchline+=decalage;
       continue;
        }
        
        line[watchline]=c;
        watchline++;
    }
    
    line[watchline]='\n';


    size_t bufsize = ARGS_SIZE;
    int position = 0;
    char **tokenstrss =NULL;
    tokenstrss=(char**) malloc(bufsize * sizeof(char**));
    char *token=NULL, **tokens_backup=NULL;

    if (!tokenstrss) {
        fprintf(stderr, " allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIM);
    while (token != NULL) {
        tokenstrss[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += ARGS_SIZE;
            tokens_backup = tokenstrss;
            tokenstrss = realloc(tokenstrss, bufsize * sizeof(char*));
            if (!tokenstrss) {
                free(tokens_backup);
                fprintf(stderr, " allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
		
        token = strtok(NULL, DELIM);
    }
    tokenstrss[position] = NULL;
    for (int i = 0; i < position ; i++)
	{
		if (!strcmp( tokenstrss[i],">>"))
		{
			iswriten=1;
			
			strcpy(filewrite,tokenstrss[i+1]);
			
			tokenstrss[i+1]=NULL;
			tokenstrss[i]=NULL;
			
			writentype=1;
			i++;	
		}else if (!strcmp( tokenstrss[i],">"))
		
		{
			iswriten=1;
			
			strcpy(filewrite,tokenstrss[i+1]);
			writentype=2;
			tokenstrss[i+1]=NULL;
			tokenstrss[i]=NULL;	
			i++;
		}else if (!strcmp( tokenstrss[i],"<"))
		{
			isreading=1;
			
			strcpy(fileread,tokenstrss[i+1]);
			tokenstrss[i+1]=NULL;
			tokenstrss[i]=NULL;	
			i++;
		}else if (!strcmp( tokenstrss[i],"|"))
		{
			ispipe=1;
			
			pipeposition[1]=i+1;
			
			tokenstrss[i]=NULL;	
			
		}else if (!strcmp( tokenstrss[i],">&"))
		{
			iswriten=1;
			
			strcpy(filewrite,tokenstrss[i+1]);
			
			tokenstrss[i+1]=NULL;
			tokenstrss[i]=NULL;
			
			writentype=3;
			i++;	
			
		}
		

	}
	
    return tokenstrss;
}


    char *line;
    char **args;





int main()
{

      if (args )
    {
        free (args);
        args =NULL;
    }
    if (line )
    {
        free (line);
        line = NULL;// prob de SEGV géré
    }
    int status;
        
    do {
		signal(SIGINT, handler_ctrlc);
        prompt();
        	
        args = line_order(line);
        status = execute(args);
        free(line);
        free(args);
    } while (status);


    return EXIT_SUCCESS;
}
