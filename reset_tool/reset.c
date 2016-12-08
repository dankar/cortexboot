#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

void serial_flush(int fd)
{
	struct termios origtty, tty;

	tcgetattr(fd, &origtty);

	tty = origtty;

	tcsetattr(fd, TCSAFLUSH, &tty);
	tcsetattr(fd, TCSADRAIN, &origtty);
}



void set_lines(int fd, int dtr, int rts)
{
	int status;

	if(ioctl(fd, TIOCMGET, &status))
	{
		printf("ioctl failed\n");
		return;
	}

	if(dtr)
		status |= TIOCM_DTR;
	else
		status &= ~TIOCM_DTR;

	if(rts)
		status |= TIOCM_LE;
	else
		status &= ~TIOCM_LE;

	if(ioctl(fd, TIOCMSET, &status))
	{
		printf("ioctl failed\n");
		return;
	}

	if(ioctl(fd, TIOCMGET, &status))
        {
                printf("ioctl failed\n");
                return;
        }
}

int main(int argc, char *argv[])
{
	int fd;

	if(argc != 2)
	{
		printf("Usage: reset tty_device\n");
		return 1;
	}

	fd = open(argv[1], O_RDWR | O_NOCTTY );

	if(fd == -1)
	{
		printf("Unable to open device '%s'\n", argv[1]);
		return 1;
	}

	set_lines(fd, 1, 1);
	usleep(100*1000);
	//serial_flush(fd);
	usleep(100*1000);
	set_lines(fd, 0, 1);
	usleep(100*1000);
	set_lines(fd, 0, 0);
	return 0;
}
