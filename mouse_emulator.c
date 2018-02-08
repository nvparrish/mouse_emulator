#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Needed for memset

#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <fcntl.h>
#include <termios.h>

void mouseDown(int button){
    Display * display = XOpenDisplay(NULL);

    XEvent event;

    if(display == NULL){
        fprintf(stderr, "Error in acquiring the display\n");
        exit(EXIT_FAILURE);
    }

    memset(&event, 0x00, sizeof(event));

    event.type = ButtonPress;
    event.xbutton.button = button;
    event.xbutton.same_screen = True;

    XQueryPointer(display, RootWindow(display, DefaultScreen(display)),
        &event.xbutton.root, &event.xbutton.window, 
        &event.xbutton.x_root, &event.xbutton.y_root, 
        &event.xbutton.x, &event.xbutton.y,
        &event.xbutton.state);

    event.xbutton.subwindow = event.xbutton.window;

    while(event.xbutton.subwindow){
        event.xbutton.window = event.xbutton.subwindow;
        XQueryPointer(display, event.xbutton.window, 
            &event.xbutton.root, &event.xbutton.subwindow, 
            &event.xbutton.x_root, &event.xbutton.y_root, 
            &event.xbutton.x, &event.xbutton.y, 
            &event.xbutton.state);
    }

    if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0)
        fprintf(stderr, "Error in sending the event.\n");

    XFlush(display);

    XCloseDisplay(display);
}

void mouseUp(int button){
    Display * display = XOpenDisplay(NULL);

    XEvent event;

    if(display == NULL){
        fprintf(stderr, "Error in acquiring the display\n");
        exit(EXIT_FAILURE);
    }

    memset(&event, 0x00, sizeof(event));

    event.type = ButtonRelease;
    event.xbutton.button = button;
    event.xbutton.same_screen = True;

    XQueryPointer(display, RootWindow(display, DefaultScreen(display)),
        &event.xbutton.root, &event.xbutton.window, 
        &event.xbutton.x_root, &event.xbutton.y_root, 
        &event.xbutton.x, &event.xbutton.y,
        &event.xbutton.state);

    event.xbutton.subwindow = event.xbutton.window;

    while(event.xbutton.subwindow){
        event.xbutton.window = event.xbutton.subwindow;
        XQueryPointer(display, event.xbutton.window, 
            &event.xbutton.root, &event.xbutton.subwindow, 
            &event.xbutton.x_root, &event.xbutton.y_root, 
            &event.xbutton.x, &event.xbutton.y, 
            &event.xbutton.state);
    }

    if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0)
        fprintf(stderr, "Error in sending the event.\n");

    XFlush(display);

    XCloseDisplay(display);
}

int mouseMoveRel(int delta_x, int delta_y){
    Display *display = XOpenDisplay(0);
    Window root = DefaultRootWindow(display);
    XWarpPointer(display, None, None, 0, 0, 0, 0, delta_x, delta_y);
    XCloseDisplay(display);
    return 0;
}

void test_command(char* buffer){
    char UP[] = "UP\n";
    char DOWN[] = "DOWN\n";
    char LEFT[] = "LEFT\n";
    char RIGHT[] = "RIGHT\n";
    if (strcmp(buffer, UP) == 0){
        //printf("Received up\n");
        mouseMoveRel(0, -5);
    } else if (strcmp(buffer, DOWN) == 0){
        //printf("Received down\n");
        mouseMoveRel(0, 5);
    } else if (strcmp(buffer, LEFT) == 0){
        //printf("Received left\n");
        mouseMoveRel(-5, 0);
    } else if (strcmp(buffer, RIGHT) == 0){
        //printf("Received right\n");
        mouseMoveRel(5, 0);
    } else if (strncmp(buffer, "MV:", 3) == 0){
        char n_16[3];
        int x, y;
        n_16[2] = '\0';
        memcpy(n_16, &(buffer[3]), 2);
        x = (int)strtol(n_16, NULL, 16);
        memcpy(n_16, &(buffer[5]), 2);
        y = (int)strtol(n_16, NULL, 16);
	if (x >= 0x80)
            x -= 0x100;
        if (y >= 0x80)
            y -= 0x100;
        mouseMoveRel(x,y);
    } else {
        printf("No match\n");
    }
}

int main(int argc, char** argv){
    struct termios acm;
    struct termios stdio;
    struct termios stdio_bak;
    int tty_fd;
    fd_set rdset;
    char buffer[20];
    char * cur_ptr = buffer;

    unsigned char c='D';

    //Get available devices

    // Select proper device

    printf("Please start with %s /dev/ttyACM0 (for example)\n", argv[0]);
    memset(&stdio, 0, sizeof(stdio));
    stdio.c_iflag = 0;
    stdio.c_oflag = 0;
    stdio.c_cflag = 0;
    stdio.c_lflag = 0;
    stdio.c_cc[VMIN] = 1;
    stdio.c_cc[VTIME] = 0;
    tcgetattr(STDOUT_FILENO, &stdio_bak);
    tcsetattr(STDOUT_FILENO, TCSANOW, &stdio);
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);   // Make the reads non-blocking

    // Set up the communication
    memset(&acm, 0, sizeof(acm));
    acm.c_iflag = 0;
    acm.c_oflag = 0;
    acm.c_cflag= CS8 | CREAD | CLOCAL;  // 8n1, see termios.h for more info
    acm.c_lflag = 0;
    acm.c_cc[VMIN]=1;
    acm.c_cc[VTIME]=5;
    // O_NONBLOCK might override VMIN and VTIME, so read() may return imm.
    tty_fd = open(argv[1], O_RDWR | O_NONBLOCK); // Read write, no blocking
    if (tty_fd < 0){
        printf("Error %d opening %s\n", tty_fd, argv[1]);
        exit(1);
    }
    printf("Got back tty_fd %x\n", tty_fd);
    cfsetospeed(&acm, B115200); // 115200 baud
    cfsetispeed(&acm, B115200); // 115200 baud

    tcsetattr(tty_fd, TCSANOW, &acm);
    while (c != 'q'){
        if (read(tty_fd, &c, 1) > 0){
            write(STDOUT_FILENO, &c, 1);
            *cur_ptr = c;
            cur_ptr++;
            if(c == '\n'){
                *cur_ptr = '\0';
                cur_ptr = buffer;
                test_command(buffer);
            }
        }
        if (read(STDIN_FILENO, &c, 1) > 0)
            write(tty_fd, &c, 1);
    }

    tcsetattr(STDOUT_FILENO, TCSANOW, &stdio_bak);
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio_bak);
    printf("\n");

    close(tty_fd);

    return 0;
}
