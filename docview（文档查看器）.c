#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_LINES 10000
#define LINE_SIZE 2048

char *lines[MAX_LINES];
int total_lines = 0;
int top_line = 0;
int console_height;

const char *text_extensions[] = {
    ".txt", ".md", ".csv", ".log", ".ini", ".cfg", ".conf",
    ".json", ".xml", ".html", ".htm", ".css", ".js", ".ts",
    ".c", ".cpp", ".cc", ".cxx", ".h", ".hpp", ".hxx",
    ".py", ".java", ".rb", ".php", ".pl", ".pm",
    ".yaml", ".yml", ".toml",
    ".bat", ".cmd", ".sh", ".bash", ".ps1", ".vbs",
    ".lua", ".go", ".rs", ".swift", ".kt", ".scala",
    ".sql", ".r", ".m", ".mm",
    ".tex", ".rst", ".asciidoc", ".adoc",
    ".properties", ".env", ".gitignore", ".dockerfile",
    ".makefile", ".cmake", ".gradle",
    NULL
};

int is_text_extension(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    for (int i = 0; text_extensions[i]; i++) {
        if (_stricmp(ext, text_extensions[i]) == 0) return 1;
    }
    return 0;
}

int is_binary(FILE *fp) {
    int c;
    int count = 0;
    while ((c = fgetc(fp)) != EOF && count < 1024) {
        if (c == 0) { fclose(fp); return 1; }
        count++;
    }
    rewind(fp);
    return 0;
}

void load_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("无法打开文件: %s\n", filename);
        exit(1);
    }

    if (!is_text_extension(filename)) {
        printf("警告: 未知文件格式 \"%s\"，尝试以文本方式打开...\n", filename);
    }

    if (is_binary(fp)) {
        printf("错误: 文件似乎是二进制文件，无法查看\n");
        fclose(fp);
        exit(1);
    }

    char buf[LINE_SIZE];
    while (fgets(buf, LINE_SIZE, fp) && total_lines < MAX_LINES) {
        buf[strcspn(buf, "\r\n")] = '\0';
        lines[total_lines] = _strdup(buf);
        total_lines++;
    }
    fclose(fp);
}

void display_page() {
    system("cls");
    COORD pos = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

    for (int i = top_line; i < top_line + console_height && i < total_lines; i++) {
        printf("%5d│ %s\n", i + 1, lines[i]);
    }

    int bottom = min(top_line + console_height, total_lines);
    printf("\n─── 行 %d-%d / 共 %d 行 ───\n", top_line + 1, bottom, total_lines);
    printf("[↑↓滚动  PgUp/PgDn翻页  /关键词搜索  Home/End首尾  q退出]\n");
}

void search(const char *keyword) {
    for (int i = top_line + 1; i < total_lines; i++) {
        if (strstr(lines[i], keyword)) {
            top_line = max(0, i - console_height / 2);
            display_page();
            return;
        }
    }
    printf("未找到: %s\n", keyword);
    Sleep(1500);
}

void goto_top() {
    top_line = 0;
}

void goto_bottom() {
    top_line = max(0, total_lines - console_height);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("用法: docview <文件名>\n");
        printf("支持的格式: txt md csv log ini cfg conf json xml html css js ts\n");
        printf("             c cpp cc cxx h hpp hxx py java rb php pl pm\n");
        printf("             yaml yml toml bat cmd sh bash ps1 vbs lua go rs\n");
        printf("             swift kt scala sql r tex rst properties env ...\n");
        return 1;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    console_height = csbi.srWindow.Bottom - csbi.srWindow.Top - 3;

    load_file(argv[1]);

    while (1) {
        display_page();
        int ch = _getch();

        if (ch == 0xE0 || ch == 0) {
            ch = _getch();
            switch (ch) {
                case 72: // ↑
                    if (top_line > 0) top_line--;
                    break;
                case 80: // ↓
                    if (top_line < total_lines - console_height) top_line++;
                    break;
                case 73: // PgUp
                    top_line = max(0, top_line - console_height);
                    break;
                case 81: // PgDn
                    top_line = min(total_lines - console_height, top_line + console_height);
                    break;
                case 71: // Home
                    goto_top();
                    break;
                case 79: // End
                    goto_bottom();
                    break;
            }
        } else if (ch == '/') {
            printf("搜索: ");
            char keyword[256];
            fflush(stdin);
            fgets(keyword, 256, stdin);
            keyword[strcspn(keyword, "\n")] = '\0';
            if (strlen(keyword) > 0) search(keyword);
        } else if (ch == 'q' || ch == 'Q') {
            break;
        }
    }

    for (int i = 0; i < total_lines; i++) free(lines[i]);
    return 0;
}