#include "interpreter.h"
#include "ftputils.h"

int interpret_comand(char *_command)
{
    /*
    Convert command into command ID
    @param _command command from client
    */
    int output;
    if(!strcmp(_command, "USER")){
        output = USER;
    }
    else if (!strcmp(_command, "PASS"))
    {
        output = PASS;
    }
    else if (!strcmp(_command, "PORT"))
    {
        output = PORT;
    }
    else if (!strcmp(_command, "PASV"))
    {
        output = PASV;
    }
    else if (!strcmp(_command, "RETR"))
    {
        output = RETR;
    }
    else if (!strcmp(_command, "STOR"))
    {
        output = STOR;
    }
    else if (!strcmp(_command, "SYST"))
    {
        output = SYST;
    }
    else if (!strcmp(_command, "TYPE"))
    {
        output = TYPE;
    }
    else if (!strcmp(_command, "QUIT"))
    {
        output = QUIT;
    }
    else if (!strcmp(_command, "MKD"))
    {
        output = MKD;
    }
    else if (!strcmp(_command, "CWD"))
    {
        output = CWD;
    }
    else if (!strcmp(_command, "PWD"))
    {
        output = PWD;
    }
    else if (!strcmp(_command, "LIST"))
    {
        output = LIST;
    }
    else if (!strcmp(_command, "RMD"))
    {
        output = RMD;
    }
    else if (!strcmp(_command, "RNFR"))
    {
        output = RNFR;
    }
    else if (!strcmp(_command, "RNTO"))
    {
        output = RNTO;
    }
    else{
        output = ERROR;
    }
    return output;
}

int interpret(int client_fd){
    int output;
    char in_cpy[MAX_BUF];
    strcpy(in_cpy, in_buf);
    char *command = strtok(in_cpy, " ");
    int cmd = interpret_comand(command);
    char msg[MAX_BUF];
    switch (client_status[client_fd])
    {
    case DISCONNECTED:
        printf("There is something wrong with server!\n");
        break;
    case CONNECTED:
        switch (cmd)
        {
        case USER:
            if(strlen(in_buf) < 5){
                strcpy(msg, "501 Please input like \"USER anonymous\"!\r\n");
                send_msg(msg, client_fd, strlen(msg));
            }
            if(user(in_buf, client_fd)){
                strcpy(msg, "504 Username not found!\r\n");
                send_msg(msg, client_fd, strlen(msg));
            }
            else{
                strcpy(msg, "331 Guest login ok, send your complete e-mail address as password.\r\n");
                send_msg(msg, client_fd, strlen(msg));
            }
            break;
        case ERROR:
            strcpy(msg, "500 You can only send command in the list!\r\n");
            send_msg(msg, client_fd, strlen(msg));
            break;
        default:
            strcpy(msg, "503 You can only send command USER before logging in!\r\n");
            send_msg(msg, client_fd, strlen(msg));
            break;
        }
        break;
    case PASSWORD:
        switch (cmd)
        {
        case PASS:
            if(strlen(in_buf) < 5){
                strcpy(msg, "501 Please input like \"PASS thu@beijing.cn\"!\r\n");
                send_msg(msg, client_fd, strlen(msg));
            }
            if(pass(in_buf, client_fd)){
                strcpy(msg, "504 Password not a email address!\r\n");
                send_msg(msg, client_fd, strlen(msg));
            }
            else{
                strcpy(msg, "230 Guest login ok, access restrictions apply.\r\n");
                send_msg(msg, client_fd, strlen(msg));
            }
            break;
        case ERROR:
            strcpy(msg, "500 You can only send command in the list!\r\n");
            send_msg(msg, client_fd, strlen(msg));
            break;
        default:
            strcpy(msg, "503 You can only send command PASS before logging in!\r\n");
            send_msg(msg, client_fd, strlen(msg));
            break;
        }
        break;
    case AVAILAVLE:
        switch(cmd){
            case SYST:
                if(strlen(in_buf) > 4){
                    strcpy(msg, "No argument needed for \"SYST\"!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else{
                    strcpy(msg, "215 UNIX Type: L8\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                break;
            case TYPE:
                if(strlen(in_buf) != 6){
                    strcpy(msg, "501 Please input like \"TYPE I\" or \"TYPE A\"!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else{
                    switch(type(in_buf, client_fd)){
                        case ASCII:
                            strcpy(msg, "200 Type set to A.\r\n");
                            send_msg(msg, client_fd, strlen(msg));
                            break;
                        case BINARY:
                            strcpy(msg, "200 Type set to I.\r\n");
                            send_msg(msg, client_fd, strlen(msg));
                            break;
                        default:
                            strcpy(msg, "504 Please input like \"TYPE I\" or \"TYPE A\"!\r\n");
                            send_msg(msg, client_fd, strlen(msg));
                    }
                }
                break;
            case PORT:
                if(port(in_buf, client_fd)){
                    strcpy(msg, "504 Invalid input!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else{
                    strcpy(msg, "200 PORT command successful.\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                break;
            case PASV:
                output = pasv(in_buf, client_fd); 
                if(output == -1){
                    strcpy(msg, "504 Invalid input!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else if(output == -2){
                    strcpy(msg, "502 Server error!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else{
                    send_msg(out_buf, client_fd, strlen(out_buf));
                }
                break;
            case RETR:
                output = retr(in_buf, client_fd);
                if(output == -1){
                    strcpy(msg, "504 Invalid Input!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else if(output == -2){
                    strcpy(msg, "502 Server Error!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else if(output == -3){
                    strcpy(msg, "245 Unable to Connect!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else{
                    strcpy(msg, "226 Sending Data Success\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                break;
            case STOR:
                output = stor(in_buf, client_fd);
                if(output == -1){
                    strcpy(msg, "504 Invalid input!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else if(output == -2){
                    strcpy(msg, "502 Server error!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else if(output == -3){
                    strcpy(msg, "245 Unable to Connect!\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                else{
                    strcpy(msg, "226 Transfer complete.\r\n");
                    send_msg(msg, client_fd, strlen(msg));
                }
                break;
            default:
                strcpy(msg, "500 You can only send command in the list!\r\n");
                send_msg(msg, client_fd, strlen(msg));
                break;
        }
        break;
    default:
        break;
    }
    
    return 0;
}