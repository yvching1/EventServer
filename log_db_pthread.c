// vi: set sw=4 ts=4:
#include "datafeedd.h"
extern struct ring_buf_t        ring_buf[MAX_RINGBUF];
MYSQL				mysql_handle ;

void write_db_dbopen()					// TODO
{
	   if ( g_bMySQLInited ){
                mysql_close( &mysql_handle );
                g_bMySQLInited = -1;
        }
        if ( !mysql_init( &mysql_handle ) )     {
                syslog (LOG_INFO, "mysqlMRP init error!");
                exit(-1);
        }

        if ( !mysql_real_connect( &mysql_handle , "localhost", g_szMySQLUser, 
			g_szMySQLPasswd, g_szMySQLDb, g_nMySQLPort, NULL, 0 ) ) {
				openlog(__progname, LOG_PID, LOG_LOCAL6);
                syslog (LOG_INFO, "mysqlMRP connect error!");
                mysql_close( &mysql_handle );
                g_bMySQLInited = -1;
                exit(-1);
        }
 	if ( mysql_set_character_set( &mysql_handle , "utf8" ) ) { 
		openlog(__progname, LOG_PID, LOG_LOCAL6);
		syslog (LOG_INFO, "utf8 error!"); 
	} 		
} // sample1_dbopen()

void
do_sql_query(char *sql_str_mysql, int sql_len, char *Eventlog){
	char			buft[512];
	MYSQL_RES		*result;

    if (prog_debug > 1)
        printf("\n sql: %s \n", sql_str_mysql);

	sprintf(buft,"sql: %s \n", Eventlog);
    openlog(__progname, LOG_PID, LOG_LOCAL6);
    syslog(LOG_INFO,buft);
    closelog();

	if ( mysql_real_query( &mysql_handle, sql_str_mysql, sql_len ) == 0 ){
		result = mysql_store_result( &mysql_handle );
		if ( result != NULL ){
			mysql_free_result( result );
 			result = NULL;
		}
	}

}
//$AEname=array("上班刷卡","上班酒測","下班刷卡","下班酒測","上班拒測","下班拒測");
//char szSQLInserToAEStatus[]="INSERT INTO `drv_red` ( `mode` , `logtime , `workno` , `mkey` , `num`, `logupdate`, `data`,`remask`) VALUES ( '\%s', '\%s', '\%s', '\%s', '\%s', '\%s','\%s','\%s');";
//AE02 20130624075555 030307007 B000210 0.000 20130624075555.jpg 02
void AE01(int argc, int ring_nr){
	int		found, sql_str_len;
	char	sql_str_mysql[512],Eventlog[100];
	char	szSQLInserToAE01[]="INSERT INTO `drv_red` ( `mode` , `logtime` , `workno` , `mkey` , `num`, `data`,`area`,`remask`) VALUES ( 'AE01', '%s', '%s', '%s', '%s', '%s','%s','上班刷卡');";
	char	AEdata[]="AE01 %s %s %s %s %s %s";
	
	memset(&Eventlog,'\0', sizeof(Eventlog));
	if (argc == 6){
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],ring_buf[ring_nr].argv[5]);
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE01, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,ring_buf[ring_nr].argv[5]);
	} else { 
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],"03");
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE01, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,"03");
	}	
	do_sql_query(sql_str_mysql, sql_str_len, Eventlog);
}//AE
void AE02(int argc, int ring_nr){
	int		found, sql_str_len;
	char	sql_str_mysql[512],Eventlog[100];
	char	szSQLInserToAE02[]="INSERT INTO `drv_red` ( `mode` , `logtime` , `workno` , `mkey` , `num`,`data`,`area`, `remask`) VALUES ( 'AE02', '%s', '%s', '%s', '%s','%s','%s','上班酒測');";
	char	AEdata[]="AE02 %s %s %s %s %s %s";
	
	memset(&Eventlog,'\0', sizeof(Eventlog));
	if (argc == 6){
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],ring_buf[ring_nr].argv[5]);
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE02, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,ring_buf[ring_nr].argv[5]);
	} else {
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],"03");
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE02, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,"03");
	}
	do_sql_query(sql_str_mysql, sql_str_len, Eventlog);
}//AE
void AE03(int argc, int ring_nr){
	int		found, sql_str_len;
	char	sql_str_mysql[512],Eventlog[100];
	char	szSQLInserToAE03[]="INSERT INTO `drv_red` ( `mode` , `logtime` , `workno` , `mkey` , `num`, `data`,`area`,`remask`) VALUES ( 'AE03', '%s', '%s', '%s', '%s', '%s','%s','下班刷卡');";
	char	AEdata[]="AE03 %s %s %s %s %s %s";

	memset(&Eventlog,'\0', sizeof(Eventlog));
	if (argc == 6){
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],ring_buf[ring_nr].argv[5]);
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE03, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,ring_buf[ring_nr].argv[5]);
	} else {
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],"03");
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE03, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,"03");
	}	
	do_sql_query(sql_str_mysql, sql_str_len, Eventlog);

}//AE
void AE04(int argc, int ring_nr){
	int		found, sql_str_len;
	char	sql_str_mysql[512],Eventlog[100];
	char	szSQLInserToAE04[]="INSERT INTO `drv_red` ( `mode` , `logtime` , `workno` , `mkey` , `num`, `data`,`area`,`remask`) VALUES ( 'AE04', '%s', '%s', '%s', '%s','%s','%s', '下班酒測');";
	char	AEdata[]="AE04 %s %s %s %s %s %s";
	
	memset(&Eventlog,'\0', sizeof(Eventlog));
	if (argc == 6){	
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],ring_buf[ring_nr].argv[5]);
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE04, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3],Eventlog,ring_buf[ring_nr].argv[5]);
	} else {
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],"03");
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE04, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3],Eventlog,"03");
	}	
	do_sql_query(sql_str_mysql, sql_str_len, Eventlog);

}//AE
void AE05(int argc, int ring_nr){
	int		found, sql_str_len;
	char	sql_str_mysql[512],Eventlog[100];
	char	szSQLInserToAE05[]="INSERT INTO `drv_red` ( `mode` , `logtime` , `workno` , `mkey` , `num`, `data`,`area`,`remask`) VALUES ( 'AE05', '%s', '%s', '%s', '%s', '%s','%s','上班拒測');";
	char	AEdata[]="AE05 %s %s %s %s %s %s";
	
	memset(&Eventlog,'\0', sizeof(Eventlog));
	if (argc == 6){	
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],ring_buf[ring_nr].argv[5]);
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE05, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,ring_buf[ring_nr].argv[5]);
	} else {
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],"03");
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE05, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,"03");
	}			
	do_sql_query(sql_str_mysql, sql_str_len, Eventlog);
	//call quota.cgi
    //kSyslog[i].len = sprintf (kSyslog[i].tx,"wget 'http://127.0.0.1/cgi-bin/quota.cgi?camid=%s&nvr=%s' -O /dev/null", deviceid, argv[3]);
	//system(kSyslog[i].tx);
	//do_live_cgi(i);
}//AE05

void AE06(int argc, int ring_nr){
	int		found, sql_str_len;
	char	sql_str_mysql[512],Eventlog[100];
	char	szSQLInserToAE06[]="INSERT INTO `drv_red` ( `mode` , `logtime` , `workno` , `mkey` , `num`, `data`,`area`,`remask`) VALUES ( 'AE06', '%s', '%s', '%s', '%s', '%s','%s','下班拒測');";
	char	AEdata[]="AE06 %s %s %s %s %s %s";
	
	memset(&Eventlog,'\0', sizeof(Eventlog));
	if (argc == 6){
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],ring_buf[ring_nr].argv[5]);
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE06, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,ring_buf[ring_nr].argv[5]);
	} else {
		sprintf (Eventlog, AEdata, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1],ring_buf[ring_nr].argv[2], ring_buf[ring_nr].argv[3], ring_buf[ring_nr].argv[4],"03");
		sql_str_len = sprintf (sql_str_mysql, szSQLInserToAE06, ring_buf[ring_nr].argv[0], ring_buf[ring_nr].argv[1], ring_buf[ring_nr].argv[2],ring_buf[ring_nr].argv[3], Eventlog,"03");
	}			
	do_sql_query(sql_str_mysql, sql_str_len, Eventlog);
	//kSyslog[i].len = sprintf (kSyslog[i].tx,"wget 'http://127.0.0.1/cgi-bin/quota.cgi?camid=%s&nvr=%s' -O /dev/null", deviceid, argv[3]);
	//system(kSyslog[i].tx);
	//do_live_cgi(i);
}//AE

void write_db_process(int ring_nr)		// TODO
{
	int keyword_nr = ring_buf[ring_nr].keyword_nr;
	int argc = ring_buf[ring_nr].argc;
	int i;
	
	write_db_dbopen();
	
	switch (keyword_nr) {
		case keyword_AE01:
			AE01(argc, ring_nr);
			break;
        case keyword_AE02:
            AE02(argc, ring_nr);
            break;
        case keyword_AE03:
            AE03(argc, ring_nr);
            break;
        case keyword_AE04:
            AE04(argc, ring_nr);
            break;
        case keyword_AE05:
            AE05(argc, ring_nr);
            break;
        case keyword_AE06:
            AE06(argc, ring_nr);
            break;
	}
} // write_db_process()


void write_db_dbclose()					// TODO
{
	mysql_close( &mysql_handle );
	g_bMySQLInited = -1;
} // write_db_dbclose()

void *thread_write_db(void *arg)
{
	int ring_pointer = 0, ring_in_now;

	write_db_dbopen();

	while (!prog_stop) {

		// waitting for ring buffer data coming
		pthread_mutex_lock (&ring_in_mutex);
		while (!prog_stop && ring_in_pointer == ring_pointer)
        	pthread_cond_wait (&ring_in_done, &ring_in_mutex);
		ring_in_now = ring_in_pointer;
    	pthread_mutex_unlock (&ring_in_mutex);
		if (prog_stop)
			break;

		// process it
		write_db_process(ring_pointer);

		// next
		pthread_mutex_lock (&ring_out_mutex);
		if ((--ring_buf[ring_pointer].thread_ref) <= 0) {
			// the slowest threads to update the ring_out
			ring_out_pointer = ring_pointer = (ring_pointer + 1) % MAX_RINGBUF;
			if (prog_debug > 1)
				printf ("thread_sample1(): move ring_out=%d (ring_in=%d)\n",
					ring_out_pointer, ring_in_now);
			pthread_cond_signal (&ring_out_done);
		} else
			ring_pointer = (ring_pointer + 1) % MAX_RINGBUF;
    	pthread_mutex_unlock (&ring_out_mutex);

	} // while()
	write_db_dbclose();

	if (prog_debug)
		printf ("thread_write_db(): end.\n");
} // thread_write_db()

