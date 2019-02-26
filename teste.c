#include <stdio.h>

int main(int argc, char **argv) {
    FILE *fp = popen("ls", "w");
    int i = 0;

    for(i=0;i<10;i++) {
        fprintf(fp, "");
    }

    pclose(fp);

    return 0;
}
