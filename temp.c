// #include<stdio.h>
// #include<sys/types.h>
// #include<sys/stat.h>
// #include <fcntl.h>  

// // chdir function is declared
// // inside this header
// #include<unistd.h>
// int main()
// {
// 	char s[100];

// 	// printing current working directory
// 	printf("%s\n", getcwd(s, 100));

// 	// using the command
// 	int changed = chdir("/home/vanshita/Desktop");
// 	printf("chenged = %d\n", changed);
// 	int fd = open("P.txt", O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IXUSR);
// 	printf("%d\n", fd);
// 	// printing current working directory
// 	printf("%s\n", getcwd(s, 100));
// 	close(fd);
// 	// after chdir is executed
// 	return 0;
// }


#include <stdio.h>
#include <dirent.h>

int main(void)
{
	struct dirent *de; // Pointer for directory entry

	// opendir() returns a pointer of DIR type.
	DIR *dr = opendir(".");

	if (dr == NULL) // opendir returns NULL if couldn't open directory
	{
		printf("Could not open current directory" );
		return 0;
	}

	// Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
	// for readdir()
	while ((de = readdir(dr)) != NULL)
			printf("%s\n", de->d_name);

	closedir(dr);	
	return 0;
}
