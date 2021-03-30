#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define TITLE_STRING  "title"
#define ARTIST_STRING "artist"
#define ALBUM_STRING  "album"
#define YEAR_STRING   "year"

typedef enum Sizes
{
	TAG_SIZE    = 3,
	YEAR_SIZE   = 4,
	ARTIST_SIZE = 30,
	TITLE_SIZE  = 30,
	ALBUM_SIZE  = 30,
	eMaxSize    = 31
} eSizes;

typedef enum Offsets
{
	TAG_OFFSET    = -128,
	YEAR_OFFSET   = -35,
	TITLE_OFFSET  = -125,
	ARTIST_OFFSET = -95,
	ALBUM_OFFSET  = -65,
	eMaxOffset    = 0
} eOffsets;

typedef struct Message
{
	char flag;
	char msg_buf[eMaxSize];
} sPackage;

#define NMR_METADATA_FIELDS 5
#define METADATA_SIZE       NMR_METADATA_FIELDS * eMaxSize
#define FIFO_NAME           "metadata_fifo"
#define RESPONSE_LEN         30

void getTitle(const int, char*);
void getArtist(const int, char*);
void getAlbum(const int, char*);
void getYear(const int, char*);

static char buffer[eMaxSize] = {0};
static char mp3_data[METADATA_SIZE] = {0};
static char reponse_buf[RESPONSE_LEN] = {0};
static char *response = "Information printed!\n";

int main(int argc, char **argv)
{	
	pid_t childId = 0;
	int named_pipe = 0;

	/* Create the FIFO (named pipe) - it will be executed
	for child and parent - the first one, who executes this
	code will create it. The second one will fail, but this
	is not an issue, because it's already in the file system.
	*/
    (void) mkfifo(FIFO_NAME, 0666);

	childId = fork();

	if (0 == childId)
	{
		sPackage message    = {0};
		named_pipe = open(FIFO_NAME, O_RDONLY);
		for(;;)
		{
			(void) read(named_pipe, &message, sizeof(message));
			(void) strcat(mp3_data, message.msg_buf);

			if (1 == message.flag)
			{
				(void) printf("\n %s \n", mp3_data);
				close(named_pipe);
				break;
			}
		}

	}
	else if (0 < childId)
	{
		if (argc >= 1)
		{
			int fd = open(argv[1], O_RDONLY);

			if (0 <= fd)
			{
				named_pipe = open(FIFO_NAME, O_WRONLY);
				sPackage message    = {0};
				int      iterator   = 0;

				for (iterator = 2; iterator < argc; iterator++)
				{
					// If input parameters for the metadata are provided:
					if (strcmp(argv[iterator], TITLE_STRING) == 0)
					{
						getTitle(fd, message.msg_buf);	
					}
					else if (strcmp(argv[iterator], ARTIST_STRING) == 0)
					{
						getArtist(fd, message.msg_buf);
					}
					else if (strcmp(argv[iterator], ALBUM_STRING) == 0)
					{
						getAlbum(fd, message.msg_buf);
					}
					else if (strcmp(argv[iterator], YEAR_STRING) == 0)
					{
						getYear(fd, message.msg_buf);
					}
					else
					{
						(void) printf("Parameter %d is unknown\n", iterator);
					}

					/* Send the content. */
					(void) write(named_pipe, &message, sizeof(message));

					// Reinitialize the message to 0.
					memset(&message, 0, sizeof(message));
				}

				message.flag = 1;
				memset(message.msg_buf, 0, sizeof(eMaxSize));

				/* Send the content. */
				(void) write(named_pipe, &message, sizeof(message));

				(void) close(fd);
				(void) close(named_pipe);
			}
		}
		else
		{
			(void) printf("Insufficient number of parameters\n");
		}
	}
	else
	{
		printf("Fork failed!");
	}

	(void) unlink(FIFO_NAME);

	return 0;
}

void getTitle(const int fd, char* buffer)
{
	// Get the offset byte.
	(void) lseek(fd, TITLE_OFFSET, SEEK_END);

	// Read the data and put into buffer.
	(void) read(fd, buffer, sizeof(char) * TITLE_SIZE);

	// Remove the white-space characters.
	(void) strtok(buffer, " ");
}

void getArtist(const int fd, char* buffer)
{
	// Get the offset byte.
	(void) lseek(fd, ARTIST_OFFSET, SEEK_END);

	// Read the data and put into buffer.
	(void) read(fd, buffer, sizeof(char) * ARTIST_SIZE);

	// Remove the white-space characters.
	(void) strtok(buffer, " ");
}

void getAlbum(const int fd, char* buffer)
{
	// Get the offset byte.
	(void) lseek(fd, ALBUM_OFFSET, SEEK_END);

	// Read the data and put into buffer.
	(void) read(fd, buffer, sizeof(char) * ALBUM_SIZE);

	// Remove the white-space characters.
	(void) strtok(buffer, " ");
}

void getYear(const int fd, char* buffer)
{
	// Get the offset byte.
	(void) lseek(fd, YEAR_OFFSET, SEEK_END);

	// Read the data and put into buffer.
	(void) read(fd, buffer, sizeof(char) * YEAR_SIZE);

	// Remove the white-space characters.
	(void) strtok(buffer, " ");
}