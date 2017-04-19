/*
 * Interval-timer demo program.
 * Hebrew University OS course.
 * Questions: os@cs.huji.ac.il
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>


int gotit = 0;
static int y = 0;
void timer_handler(int sig)
{
  gotit = 1;
  int j;
  printf("Timer expired %d\n",y);
}

static void dummy_func(int j){
	return;
}

int main(void) {
	struct sigaction sa;
	struct itimerval timer;

	// Install timer_handler as the signal handler for SIGVTALRM.
	sa.sa_handler = &timer_handler;
	if (sigaction(SIGVTALRM, &sa,NULL) < 0) {
		printf("sigaction error.");
	}

	// Configure the timer to expire after 1 sec... */
	timer.it_value.tv_sec = 1;		// first time interval, seconds part
	timer.it_value.tv_usec = 0;		// first time interval, microseconds part

	// configure the timer to expire every 3 sec after that.
	timer.it_interval.tv_sec = 5;	// following time intervals, seconds part
	timer.it_interval.tv_usec = 0;	// following time intervals, microseconds part

	// Start a virtual timer. It counts down whenever this process is executing.
	if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
		printf("setitimer error.");
	}
	sigset_t sig_vt_alarm_set, waiting_sig_mask;
	sigemptyset(&sig_vt_alarm_set);
    sigaddset(&sig_vt_alarm_set, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &sig_vt_alarm_set, NULL);

    // with waiting option 
    sigemptyset(&waiting_sig_mask);
    // sigaddset(&waiting_sig_mask, SIGVTALRM);
    // raise(SIGVTALRM);
    /*
    for (int i,j =0; i<100000000; i++){
    	j = j * j;
    	dummy_func(j);
    }
    */
    while(!sigismember(&waiting_sig_mask, SIGVTALRM)){
    	    sigpending(&waiting_sig_mask);
    }
    // raise(SIGVTALRM);
    int k ,l ,m, p;
	y = 5;
	// USING the waiting option..
	if (sigismember(&waiting_sig_mask, SIGVTALRM)) {
		printf("%s\n","test1010\n");
		y=7;
        int dummy_sig_num_int_pointer;
        sigwait(&waiting_sig_mask, &dummy_sig_num_int_pointer);
    }
    sigprocmask(SIG_UNBLOCK, &sig_vt_alarm_set, NULL);
    // y = 9;
	for(;;) {

		if (gotit) {
			printf("Got it!\n");
			gotit = 0;
		}
	}
}
