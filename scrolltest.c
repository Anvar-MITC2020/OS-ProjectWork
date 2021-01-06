#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <my_global.h>
#include <mysql.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define PORT 8888
#define BUFSIZE 1024

char key,key1;
static char *host = "localhost";
static char *user = "root";
static char *passwd = "root";
static char *dbname = "dynamicbuslines";
unsigned int port = 3306; //port number for the MySQL db
static char *unix_socket = NULL;
unsigned int flag = 0; //connection ODBC
char buf[100];
char query[100];
char *resulted[100];
char value1[100]="default";
char value2[100]="default";
int firstvalue=1;
  gchar *ColumnNames[100];
  int counter=0,counter1=0;
MYSQL *connection;
GtkBuilder *builder;
GtkWidget *bus;
GtkWidget *window;
GtkWidget *scwin;
  GtkWidget *view;
  GtkWidget *show_tables;
  GtkWidget *button;
  GtkWidget *grid;
  GtkWidget *box;

  void insert_into_takes()
  {
    printf("INSERTTING INTO DATABSE\n");
  sprintf(query,"INSERT INTO takes VALUES('%s','%s','stop117','stop118')",value1,value2);
  mysql_query(connection,query);
  }

void send_to_all(int j, int i, int sockfd, int nbytes_recvd, char *recv_buf, fd_set *master)
{
  if (FD_ISSET(j, master)){
    if (j != sockfd && j != i) {
      if (send(j, recv_buf, nbytes_recvd, 0) == -1) {
        perror("send");
      }
    }
  }
}

void send_recv(int i, fd_set *master, int sockfd, int fdmax)
{
  int nbytes_recvd, j;
  char recv_buf[BUFSIZE], buf[BUFSIZE];
  
  if ((nbytes_recvd = recv(i, recv_buf, BUFSIZE, 0)) <= 0) {
    if (nbytes_recvd == 0) 
    {
      insert_into_takes();
      printf("socket %d  %s %s hung  up\n ", i,value1,value2);

    }
    else {
      perror("recv");
    }
    close(i);
    FD_CLR(i, master);
  }else 
  { 
  recv_buf[nbytes_recvd] = '\0';
     printf("%s\n", recv_buf);
     for (int i = 0; i < 10; ++i)
     {
      value1[i]=recv_buf[i];
       /* code */
     }
     for (int i = 10; i < 18; ++i)
     {
      value2[i-10]=recv_buf[i];
       /* code */
     }
     // if(firstvalue)
     // {
     //  strcpy(value1,recv_buf);
     //  firstvalue=0;
     // }
     // else
     // {
     //  strcpy(value2,recv_buf);
     //  firstvalue=1;
     // }
    for(j = 0; j <= fdmax; j++){
      send_to_all(j, i, sockfd, nbytes_recvd, recv_buf, master );
    }
  fflush(stdout);
  }  
}
void connection_accept(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in *client_addr)
{
  socklen_t addrlen;
  int newsockfd;
  
  addrlen = sizeof(struct sockaddr_in);
  if((newsockfd = accept(sockfd, (struct sockaddr *)client_addr, &addrlen)) == -1) {
    perror("accept");
    exit(1);
  }else {
    FD_SET(newsockfd, master);
    if(newsockfd > *fdmax){
      *fdmax = newsockfd;
    }
    printf("new connection from %s on port %d \n",inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
  }
}
  
  void connect_request(int *sockfd, struct sockaddr_in *my_addr)
{
  int yes = 1;
    
  if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }
    
  my_addr->sin_family = AF_INET;
  my_addr->sin_port = htons(8888);
  my_addr->sin_addr.s_addr = INADDR_ANY;
  memset(my_addr->sin_zero, '0', sizeof my_addr->sin_zero);
    
  if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }
    
  if (bind(*sockfd, (struct sockaddr *)my_addr, sizeof(struct sockaddr)) == -1) {
    perror("Unable to bind");
    exit(1);
  }
  if (listen(*sockfd, 10) == -1) {
    perror("listen");
    exit(1);
  }
  printf("\nTCPServer Waiting for client on port 8888\n");
  fflush(stdout);
}
enum bus_stops
{
COL_BUS_STOP=0,
NUM_COLS_BUSSTOP
};


enum passenger
{
COL_PASS_ID=0,
COL_BALANCE,
COL_FREQ,
NUM_COLS_PASS
};

enum takes
{
COL_BUS_ID_TAKES=0,
COL_PASS_ID_TAKES,
COL_INIT_STOP,
COL_FIIN_STOP,
NUM_COLS_TAKES
};

enum bus_info
{
  COL_BUS_ID=0,
  COL_LINE_ID_BUSINFO,
  COL_CAP,
  COL_MODEL,
  NUM_COLS_BUSINFO
} ;

enum bus_line
{
  COL_LINE_ID_BUSLINE=0,
  COL_LENGTH,
  COL_NUM_OF_STOPS,
  COL_WEIGHT,
  NUM_COLS_BUSLINE
} ;


void on_back_clicked()
{

  gtk_widget_hide(window);
  gtk_widget_show(bus);
}


void connect_mysql()
{
  connection = mysql_init(NULL);
  printf("MySQL client version: %s\n", mysql_get_client_info());
  if(!(mysql_real_connect(connection, host, user, passwd, dbname, port, unix_socket, flag)))
  {
    fprintf(stderr, "Error: %s [%d]\n", mysql_error(connection), mysql_errno(connection));
    mysql_close(connection);
    exit(1);
  }
  else printf("connection success\n");
}

static GtkTreeModel * 
create_and_fill_model_bus_line (void)
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  

  sprintf(query,"SELECT * FROM bus_line");
  mysql_query(connection,query);
  MYSQL_RES *result = mysql_store_result(connection);
  int num_fields = mysql_num_fields(result);
  MYSQL_ROW row;
  MYSQL_FIELD *field;
 

  store = gtk_list_store_new (NUM_COLS_BUSLINE, G_TYPE_STRING, G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);

  /* Append a row and fill in some data */
   while ((row = mysql_fetch_row(result)))
  {
      gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_LINE_ID_BUSLINE, row[0],
                      COL_LENGTH, row[1],
                      COL_NUM_OF_STOPS, row[2],
                      COL_WEIGHT, row[3],
                      -1);

  }
  mysql_free_result(result);
  
 
  return GTK_TREE_MODEL (store);
}

static GtkTreeModel *
create_and_fill_model_bus_info (void)
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  

  sprintf(query,"SELECT * FROM bus_info");
  mysql_query(connection,query);
  MYSQL_RES *result = mysql_store_result(connection);
  int num_fields = mysql_num_fields(result);
  MYSQL_ROW row;
  MYSQL_FIELD *field;
 

  store = gtk_list_store_new (NUM_COLS_BUSINFO, G_TYPE_STRING, G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);

  /* Append a row and fill in some data */
   while ((row = mysql_fetch_row(result)))
  {
      gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_BUS_ID, row[0],
                      COL_LINE_ID_BUSINFO, row[1],
                      COL_CAP, row[2],
                      COL_MODEL, row[3],
                      -1);

  }
  mysql_free_result(result);
  
 
  return GTK_TREE_MODEL (store);
}

static GtkTreeModel *
create_and_fill_model_bus_stop (void)
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  

  sprintf(query,"SELECT * FROM bus_stop");
  mysql_query(connection,query);
  MYSQL_RES *result = mysql_store_result(connection);
  int num_fields = mysql_num_fields(result);
  MYSQL_ROW row;
  MYSQL_FIELD *field;
 

  store = gtk_list_store_new (NUM_COLS_BUSSTOP, G_TYPE_STRING);

  /* Append a row and fill in some data */
   while ((row = mysql_fetch_row(result)))
  {
      gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_BUS_STOP, row[0]
                      -1);

  }
  mysql_free_result(result);
  
 
  return GTK_TREE_MODEL (store);
}

static GtkTreeModel *
create_and_fill_model_passenger (void)
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  

  sprintf(query,"SELECT * FROM passenger");
  mysql_query(connection,query);
  MYSQL_RES *result = mysql_store_result(connection);
  int num_fields = mysql_num_fields(result);
  MYSQL_ROW row;
  MYSQL_FIELD *field;
 

  store = gtk_list_store_new (NUM_COLS_PASS, G_TYPE_STRING, G_TYPE_STRING,G_TYPE_STRING);

  /* Append a row and fill in some data */
   while ((row = mysql_fetch_row(result)))
  {
      gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_PASS_ID, row[0],
                      COL_BALANCE, row[1],
                      COL_FREQ, row[2],
                      -1);

  }
  mysql_free_result(result);
  
 
  return GTK_TREE_MODEL (store);
}
static GtkTreeModel *
create_and_fill_model_takes (void)
{
  GtkListStore  *store;
  GtkTreeIter    iter;
  

  sprintf(query,"SELECT * FROM takes");
  mysql_query(connection,query);
  MYSQL_RES *result = mysql_store_result(connection);
  int num_fields = mysql_num_fields(result);
  MYSQL_ROW row;
  MYSQL_FIELD *field;
 

  store = gtk_list_store_new (NUM_COLS_TAKES, G_TYPE_STRING, G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);

  /* Append a row and fill in some data */
   while ((row = mysql_fetch_row(result)))
  {
      gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_BUS_ID_TAKES, row[0],
                      COL_PASS_ID_TAKES, row[1],
                      COL_INIT_STOP, row[2],
                      COL_FIIN_STOP, row[3],
                      -1);

  }
  mysql_free_result(result);
  
 
  return GTK_TREE_MODEL (store);
}
static GtkWidget *
create_view_and_model_bus_line (void)
{
 
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "line_id",  
                                               renderer,
                                               "text", COL_LINE_ID_BUSLINE,
                                               NULL);

  
  
  /* --- Column #2 --- */

  col = gtk_tree_view_column_new();

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "length",
                                               renderer,
                                               "text", COL_LENGTH,
                                               NULL);
                                               

  col = gtk_tree_view_column_new();

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "number of stops",  
                                               renderer,
                                               "text", COL_NUM_OF_STOPS,
                                               NULL);
  col = gtk_tree_view_column_new();

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "weightload",  
                                               renderer,
                                               "text", COL_WEIGHT,
                                               NULL);
  model = create_and_fill_model_bus_line ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  g_object_unref (model); /* destroy model automatically with view */

  return view;
}

static GtkWidget *
create_view_and_model_bus_stop (void)
{
  
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "bus_stop",  
                                               renderer,
                                               "text", COL_BUS_STOP,
                                               NULL);


  model = create_and_fill_model_bus_info ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  g_object_unref (model); /* destroy model automatically with view */

  return view;
}
static GtkWidget *
create_view_and_model_takes (void)
{
  
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "bus_id",  
                                               renderer,
                                               "text", COL_BUS_ID_TAKES,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "passenger_id_id",  
                                               renderer,
                                               "text", COL_PASS_ID_TAKES,
                                               NULL);

    renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "Initial_stop",  
                                               renderer,
                                               "text", COL_INIT_STOP,
                                               NULL);
    renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "final_stop",  
                                               renderer,
                                               "text", COL_FIIN_STOP,
                                               NULL);

  model = create_and_fill_model_takes ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  g_object_unref (model); /* destroy model automatically with view */

  return view;
}
static GtkWidget *
create_view_and_model_passenger (void)
{
  
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "passenger_id",  
                                               renderer,
                                               "text", COL_PASS_ID,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "balance",  
                                               renderer,
                                               "text", COL_BALANCE,
                                               NULL);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "frequency",  
                                               renderer,
                                               "text", COL_FREQ,
                                               NULL);


  model = create_and_fill_model_passenger ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  g_object_unref (model); /* destroy model automatically with view */

  return view;
}
static GtkWidget *
create_view_and_model_bus_info (void)
{
  
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                              "bus_id",  
                                               renderer,
                                               "text", COL_BUS_ID,
                                               NULL);

  
  
  /* --- Column #2 --- */

  col = gtk_tree_view_column_new();

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "line_id",  
                                               renderer,
                                               "text", COL_LINE_ID_BUSINFO,
                                               NULL);
                                               

  col = gtk_tree_view_column_new();

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "capacity",  
                                               renderer,
                                               "text", COL_CAP,
                                               NULL);
  col = gtk_tree_view_column_new();

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "model",  
                                               renderer,
                                               "text", COL_MODEL,
                                               NULL);
  model = create_and_fill_model_bus_info ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  g_object_unref (model); /* destroy model automatically with view */

  return view;
}


int
main (int argc, char **argv)
{
  connect_mysql();
  pthread_t gui_thread;

  fd_set master;
  fd_set read_fds;
  int fdmax, i;
  int sockfd= 0;
  struct sockaddr_in my_addr, client_addr;
  
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  connect_request(&sockfd, &my_addr);
  FD_SET(sockfd, &master);
  

  gtk_init (&argc, &argv);

  scwin=gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scwin),GTK_POLICY_AUTOMATIC,GTK_POLICY_ALWAYS);
   builder = gtk_builder_new_from_file ("bus.glade");
   gtk_builder_connect_signals(builder, NULL);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window),500,250);
  g_signal_connect (window, "delete_event", gtk_main_quit, NULL); /* dirty */

  button=gtk_button_new_with_label("Back");
  g_signal_connect (button, "clicked",G_CALLBACK(on_back_clicked),NULL);

box=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
gtk_box_pack_end(GTK_BOX(box),button,FALSE,FALSE,0);

bus  = GTK_WIDGET(gtk_builder_get_object(builder, "bus"));
 g_signal_connect (bus, "delete_event", gtk_main_quit, NULL);

    gtk_widget_hide (GTK_WIDGET(bus));

    gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER_ALWAYS);

    gtk_widget_show(bus);
   
    pthread_create(&gui_thread,NULL,gtk_main,NULL);

    fdmax = sockfd;
  while(1){
    read_fds = master;
    if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
      perror("select");
      exit(4);
    }
    
    for (i = 0; i <= fdmax; i++){
      if (FD_ISSET(i, &read_fds)){
        if (i == sockfd)
          connection_accept(&master, &fdmax, sockfd, &client_addr);
        else
          send_recv(i, &master, sockfd, fdmax);
      }
    }
  }
  

   pthread_join(gui_thread,NULL);
  return 0;
}

void on_bus_id_clicked()
{gtk_widget_hide (GTK_WIDGET(bus));
  gtk_container_remove(GTK_BOX(scwin),view);
  view = create_view_and_model_bus_info ();
  gtk_window_set_title(GTK_WINDOW(window),"BUS INFO");
   gtk_container_add(GTK_CONTAINER(scwin),view);
   gtk_box_pack_start(GTK_BOX(box),scwin,TRUE,TRUE,0);
gtk_container_add(GTK_CONTAINER(window),box);
   gtk_widget_show_all (window);
}

void on_bus_line_clicked()
{gtk_widget_hide (GTK_WIDGET(bus));
gtk_container_remove(GTK_BOX(scwin),view);
  view = create_view_and_model_bus_line ();
  gtk_window_set_title(GTK_WINDOW(window),"BUS LINE");
  gtk_container_add(GTK_CONTAINER(scwin),view);
   gtk_box_pack_start(GTK_BOX(box),scwin,TRUE,TRUE,0);
gtk_container_add(GTK_CONTAINER(window),box);
   gtk_widget_show_all (window);
}

void on_bus_stop_clicked()
{gtk_widget_hide (GTK_WIDGET(bus));
gtk_container_remove(GTK_BOX(scwin),view);
  view = create_view_and_model_bus_stop ();
  gtk_window_set_title(GTK_WINDOW(window),"BUS STOPS");
gtk_container_add(GTK_CONTAINER(scwin),view);
   gtk_box_pack_start(GTK_BOX(box),scwin,TRUE,TRUE,0);
gtk_container_add(GTK_CONTAINER(window),box);
gtk_widget_show_all (window);
}
void on_passenger_clicked()
{gtk_widget_hide (GTK_WIDGET(bus));
gtk_container_remove(GTK_BOX(scwin),view);
  view = create_view_and_model_passenger ();
  gtk_window_set_title(GTK_WINDOW(window),"PASSENGERS");
gtk_container_add(GTK_CONTAINER(scwin),view);
   gtk_box_pack_start(GTK_BOX(box),scwin,TRUE,TRUE,0);
gtk_container_add(GTK_CONTAINER(window),box);
gtk_widget_show_all (window);
}
void on_takes_clicked()
{gtk_widget_hide (GTK_WIDGET(bus));
gtk_container_remove(GTK_BOX(scwin),view);
  view = create_view_and_model_takes ();
  gtk_window_set_title(GTK_WINDOW(window),"TAKES");
gtk_container_add(GTK_CONTAINER(scwin),view);
   gtk_box_pack_start(GTK_BOX(box),scwin,TRUE,TRUE,0);
gtk_container_add(GTK_CONTAINER(window),box);
gtk_widget_show_all (window);
}
