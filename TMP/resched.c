/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>


unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

void newepoch()
{
        int i=0;
        for(i = 1; i < NPROC; i++)
        {
                if(proctab[i].pstate == PRFREE)
                        continue;

                if(proctab[i].counter > 0)
                        proctab[i].counter = (proctab[i].counter >> 1) + proctab[i].pprio;
                else
                        proctab[i].counter = proctab[i].pprio;
                        proctab[i].goodness = proctab[i].counter + proctab[i].pprio;
        }
}

void realnewepoch()
{
	int i=0;
	for(i = 1; i < NPROC; i++)
	{
		if(proctab[i].pstate == PRFREE)
			continue;
		if(proctab[i].pisreal == 1)
		{
			proctab[i].counter = 100;
		}
	}
}

void normalnewepoch()
{
        int i=0;
        for(i = 1; i < NPROC; i++)
        {
                if(proctab[i].pstate == PRFREE)
                        continue;
                if(proctab[i].pisreal == 0)
                {
                	if(proctab[i].counter > 0)
                        proctab[i].counter = (proctab[i].counter >> 1) + proctab[i].pprio;
                	else
                        proctab[i].counter = proctab[i].pprio;
                        proctab[i].goodness = proctab[i].counter + proctab[i].pprio;
                }
        }
}

int isrealempty()
{
	int i = 0;
	for(i = 1; i < NPROC; i++)
	{
		if((proctab[i].pisreal == 1) && (proctab[i].pstate == PRREADY))
			return 0;
	}
	return 1;
}

int isnormalempty()
{
	int i = 0;
	for(i = 1; i < NPROC; i++)
	{
		if((proctab[i].pisreal == 0) && (proctab[i].pstate == PRREADY))
			return 0;
	}
	return 1;
}

int maximumgoodness()
{
	int nextprocess = -1;
	int i = 0;
	int maxgoodness = 0;
	for(i =1; i<NPROC; i++)
	{
		if((proctab[i].pisreal == 0) && (proctab[i].pstate == PRREADY)&&(proctab[i].goodness > maxgoodness))
		{
			maxgoodness = proctab[i].goodness;
			nextprocess = i;
		}
	}

	return nextprocess;
}

int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/
	int class = getschedclass();
	//kprintf("class selected = %d", class);
	if(class == 0)
	{

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
	}

	else if(class == 1)
	{
		//kprintf("entered linux class");
		int i = 0;

		int next = currpid;

		int isnewepoch = 1;

		int tempcounter = 0;

		optr = &proctab[currpid];
		
		if(preempt<=0)
			{
			preempt = 0;
			proctab[currpid].counter = 0;
			proctab[currpid].goodness = 0;
			}

                proctab[currpid].goodness = proctab[currpid].goodness - proctab[currpid].counter + preempt;
		
		optr->counter = preempt;

		int maxgoodness = 0;

			for(i = 0; i < NPROC; i++ )
			{
				if((proctab[i].pstate == PRREADY)&&(proctab[i].counter>0))
				{
					tempcounter = proctab[i].counter;
					isnewepoch = 0;
					break;
				}
			}

			if((isnewepoch==1) && (proctab[currpid].pstate == PRCURR) && (proctab[currpid].counter>0))
			{
				preempt = proctab[currpid].counter;
				return OK;
			}

			if(isnewepoch == 1)
			{
				newepoch();
                        }

			for(i = 0; i<NPROC; i++)
			{
				if((proctab[i].pstate == PRREADY)&&(proctab[i].goodness > maxgoodness))
				{
					maxgoodness = proctab[i].goodness;
					next = i;
				}
			}

			if((maxgoodness < proctab[currpid].goodness)&&(proctab[currpid].pstate == PRCURR))
			{
				preempt = proctab[currpid].counter;
				return OK;
			}
			else if(proctab[next].counter>0)
			{
				if (optr->pstate == PRCURR)
                        	{
                                	optr->pstate = PRREADY;
                                	insert(currpid,rdyhead,optr->pprio);
                        	}
                        	currpid = next;
                        	dequeue(next);
                        	nptr = &proctab[next];
                        	nptr->pstate = PRCURR;

                        	preempt = nptr->counter;

                        	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                        	return OK;
			}
			else
			{
				//nptr = &proctab[currpid];
				//nptr->pstate = PRCURR;
				//preempt = QUANTUM;
				//ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
				//kprintf("\n no process");
				return -1;
			}
	}
	else if (class == 2)
	{
		int i = 0;
		int num = 0;
		int isnewepoch = 0;
		int next = -1;
		int isrealrunning = 0;
		int realempty = 0;
		int normalempty = 0;
		optr = &proctab[currpid];
		realempty = isrealempty();
		normalempty = isnormalempty();
		if(preempt == 0) proctab[currpid].counter = 0;
		if(realempty && normalempty)
		{
			//kprintf("\n\t[248]returning to %d\n",currpid);
			if(proctab[currpid].counter <= 0 && proctab[currpid].pstate == PRFREE)
			{
				//currpid = NULLPROC;
				nptr = &proctab[NULLPROC];
				nptr->pstate = PRCURR;
				preempt = QUANTUM;
				//kprintf("[293]%d\n",currpid);
				ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
				//kprintf("295 %d\n", currpid);
				return OK;
			}
			else
				return OK;
		}

		if (preempt<=0)
			preempt = 0;

		if(proctab[currpid].pisreal == 0)
		{
			proctab[currpid].goodness = proctab[currpid].goodness - proctab[currpid].counter + preempt;
			isrealrunning = 0;
		}

		if (preempt == 0)
		{
			proctab[currpid].counter = 0;
			proctab[currpid].goodness = 0;
		}

		proctab[currpid].counter = preempt;

		if (proctab[currpid].pisreal == 1)
		{
			if(proctab[currpid].pstate == PRCURR && proctab[currpid].counter > 0)
			{
				preempt = proctab[currpid].counter;
				return OK;
			}
			else
			{
				////kprintf("Line 233 \n");
				for(i = 1; i < NPROC; i++)
				{
					if(proctab[i].pstate == PRREADY && proctab[i].pisreal == 1 && proctab[i].counter > 0)
					{
						if (optr->pstate == PRCURR && currpid!=NULLPROC)
						{
							optr->pstate = PRREADY;
							insert(currpid,rdyhead,optr->pprio);
						}
						currpid = i;
						dequeue(i);
						nptr = &proctab[i];
						nptr->pstate = PRCURR;
						preempt = nptr->counter;
						//kprintf("[293]%d\n",currpid);
						ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
						return OK;
					}
				}

				isnewepoch = 1;
			}
		}
		else
		{
			next = maximumgoodness();
			if(currpid == 0)
			{
				//kprintf("\n\t[307]returning to %d\n",currpid);
				return -1;
			}
			if((next == -1) && (proctab[currpid].pstate == PRCURR) && (proctab[currpid].counter > 0))
			{
				preempt = proctab[currpid].counter;
				//kprintf("\n\t[313]returning to %d\n",currpid);
				return OK;
			}

			if(next>0)
			{
				if (optr->pstate == PRCURR && currpid!=NULLPROC)
				{
					optr->pstate = PRREADY;
					insert(currpid,rdyhead,optr->pprio);
				}
				currpid = next;
				dequeue(next);
				nptr = &proctab[next];
				nptr->pstate = PRCURR;
				preempt = nptr->counter;
				//kprintf("[325]%d\n",currpid);
				ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
				return OK;
			}
			else
			{
				isnewepoch = 1;
			}
		}

		if((isnewepoch))
		{
			num = rand()%100;

			if(num>=0 && num<70)
			{
				realempty = isrealempty();
				normalempty = isnormalempty();

				if(realempty && normalempty )
				{
					//nptr = &proctab[currpid];
                                	//nptr->pstate = PRCURR;
                                	//preempt = QUANTUM;
                                	//ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
                                	//kprintf("\n no process");
					//kprintf("%d",currpid);
					//kprintf("\n\t[356]returning to %d\n",currpid);
					return -1;
				}

				if(!realempty)
				{
					realnewepoch();
					for(i = 1; i < NPROC; i++)
					{
						if(proctab[i].counter > 0)
						{
							if (optr->pstate == PRCURR)
							{
								optr->pstate = PRREADY;
								insert(currpid,rdyhead,optr->pprio);
							}
							currpid = i;
							dequeue(i);
							nptr = &proctab[i];
							nptr->pstate = PRCURR;
							preempt = nptr->counter;
							//kprintf("[372]%d\n",currpid);
							ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
							return OK;
						}
					}
					if(isrealempty() && isnormalempty())
					{
						//kprintf("\n\t[384]returning to %d\n",currpid);
						return -1;
					}
				}

				if(!normalempty)
				{
					normalnewepoch();
					next = maximumgoodness();
					if(next == -1)
					{
						//kprintf("\n\t[395]returning to %d\n",currpid);
						return -1;
					}
					if (optr->pstate == PRCURR && currpid!=NULLPROC)
					{
						optr->pstate = PRREADY;
						insert(currpid,rdyhead,optr->pprio);
					}
					currpid = next;
					dequeue(next);
					nptr = &proctab[next];
					nptr->pstate = PRCURR;
					preempt = nptr->counter;
					//kprintf("[397]%d\n",currpid);
					ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
					return OK;
				}
			}

			else
			{
				realempty = isrealempty();
				normalempty = isnormalempty();

				if(realempty && normalempty)
				{
					kprintf("\n\t[421]returning to %d\n",currpid);
					return -1;
				}

				if(!normalempty)
				{
					normalnewepoch();
					next = maximumgoodness();
					if(next == -1)
					{
						//kprintf("\n\t[431]returning to %d\n",currpid);
						return -1;
					}
					if (optr->pstate == PRCURR && currpid!=NULLPROC)
					{
						optr->pstate = PRREADY;
						insert(currpid,rdyhead,optr->pprio);
					}
					currpid = next;
					dequeue(next);
					nptr = &proctab[next];
					nptr->pstate = PRCURR;
					preempt = nptr->counter;
					////kprintf("[430]%d\n",currpid);
					ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
					return OK;
				}

				if(!realempty)
				{
					realnewepoch();
					for(i = 1; i < NPROC; i++)
					{
						if(proctab[i].counter > 0)
						{
							if (optr->pstate == PRCURR)
							{
								optr->pstate = PRREADY;
								insert(currpid,rdyhead,optr->pprio);
							}
							currpid = i;
							dequeue(i);
							nptr = &proctab[i];
							nptr->pstate = PRCURR;
							preempt = nptr->counter;
							////kprintf("[452]%d\n",currpid);
							ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
							return OK;
						}
					}
					if(isrealempty()&&isnormalempty())
					{
						//kprintf("\n\t[473]returning to %d\n",currpid);
						return -1;
					}
				}
			}
		}
	}
}
