#ifndef PRESENCE_DETECTION_H
#define PRESENCE_DETECTION_H

#define INTERVAL_LOW 100u
#define INTERVAL_HIGH 1000u
#define FRAME_RATE_LOW 1u
#define FRAME_RATE_HIGH 10u

//Radar config params - DO NOT CHANGE
#define SC1233A_RADAR_INTER_1FPS	(1000u)
#define SC1233A_RADAR_INTER_10FPS	(100u)
#define SC1233A_RADAR_INTER_BETA_1FPS	(52u)
#define SC1233A_RADAR_INTER_BETA_10FPS	(205u)
#define SC1233A_RADAR_FRAME_RATE_1FPS	(1u)
#define SC1233A_RADAR_FRAME_RATE_10FPS	(10u)


#define DISTANCE_THRESHOLD 40u
#define INGRESS_SCORE_THRESHOLD 8u

/*radar parameter structure: this includes the radar parameters that can be updated from the SNI API*/
struct parameter_radar_dynamic
{
	u16	interval;
	u8	beta;
};

/*application structure*/
struct presence_handle
{
	u8	status;			// presence detection result
	u8	frame_counter;		// frame_counter of ingress validation window
	u16	no_presence_counter;	// frame_counter of egress validation window
	u8	counter_score;		// score to validate the ingress
	u8	rate_change_flag;	// flag to control rate change
	u8	initialized;		// flag to check if presence_handle has been initialized.
};


/*algorithm parameter structure*/
struct parameter_app
{
	u8 presence_threshold;		// peak threshold
	u8 turnOn_delay;		// timewindow (# of frames) to justify ingress
	u8 trigger_outdate_counter;	// timer (# of frames) when an old ingress trigger expires
	u8 turnOff_delay;		// timewindow (# of frames) to justify egress
	u8 score_ratio;			// percentage threshold (%) to determine whether there are enough spikes to confirm ingress
};


/*change the radar parameters*/
/*intialize algorithm parameters*/
void parameter_radar_dynamic_update(struct parameter_radar_dynamic *para, u16 interval)
{
	/*change the parameters of radar configurations by the specified frame interval. Two radar parameter sets are given for two frame intervals, i.e., 100 [ms] and 1000 [ms]*/
	/*
	Input:
	para: pointer of struct parameter_radar_dynamic, which includes the tunable radar parameters
	interval: uint16_t type, the interval [ms]*/

	para->interval = interval;
	para->beta = (interval == INTERVAL_HIGH) ? 52 : 205;

}

/*intialize algorithm parameters*/
int parameter_app_update(struct parameter_app *para, u8 frame_rate)
{
	/*change the parameter set of application algorithm by the specified frame rate*/
	/*
	Input:
	para: pointer of struct parameter_app , which includes the frame rate-specified algorithm parameters
	interval: uint16_t type, the interval [ms]

	Return:
	1 or 0: 1 indicates that the radar parameter set for the specified interval is valid. 0 indicates that the radar parameter set for the specified interval is not defined.
	para will be updated by the parameter set specified by frame rate
	*/

	switch (frame_rate)
	{
	case FRAME_RATE_LOW:
		para->presence_threshold = 19;
		para->turnOn_delay = (u8)(1 * frame_rate);
		para->trigger_outdate_counter = (u8)(2 * frame_rate);
		para->turnOff_delay = (u8)(10 * frame_rate);
		para->score_ratio = 20;
		break;
	case FRAME_RATE_HIGH:
		para->presence_threshold = 23;
		para->turnOn_delay = (u8)(1 * frame_rate);
		para->trigger_outdate_counter = (u8)(15 * frame_rate/10);
		para->turnOff_delay = (u8)(5 * frame_rate);
		para->score_ratio = 20;
		break;
	default:
		return -1;
	}
	return 1;
}

/*function to initialize the presence_handle*/
void presence_init(struct presence_handle *handle)
{
	/*initialize the presence_handle with all zero values for attribute*/
	/*
	Input:
	handle: pointer of struct presence_handle

	Return:
	None
	*/
	handle->status = 0;
	handle->frame_counter = 0;
	handle->no_presence_counter = 0;
	handle->counter_score = 0;
	handle->rate_change_flag = 0;
	handle->initialized = 1;
}

/* function to fulfill the presence detection*/
void presence_detection(struct presence_handle *handle_2D, struct parameter_app *para, struct parameter_radar_dynamic *para_radar, u8 peak_level, u32 distance)
{
	/* Input:
	   handle_2D: pointer of presence_handle
	   para: pointer of parameter_app
	   data: pointer of data_frame
	   Return:
	   None
	*/

	/*check whether there exists a detection trigger to be validated*/
	if (handle_2D->frame_counter > 0)
	{
		handle_2D->frame_counter += 1;
	}

	/*no valid measurements*/
	if (peak_level < para->presence_threshold)
	{
		/*check whether the candidate detection trigger has been outdated or not*/
		if (handle_2D->status == 0 && handle_2D->frame_counter > (para->trigger_outdate_counter) + 1)
		{
			handle_2D->counter_score = 0;
			handle_2D->frame_counter = 0;
		}

		/*increment the non-presence counter when TV is on*/
		if (handle_2D->status == 1)
		{
			handle_2D->no_presence_counter += 1;
			if (handle_2D->no_presence_counter >= para->turnOff_delay)
			{
				if (para_radar->interval == INTERVAL_HIGH) {
					handle_2D->status = 0;
					parameter_radar_dynamic_update(para_radar, INTERVAL_LOW);
					parameter_app_update(para, FRAME_RATE_HIGH);
				}
				else {
					parameter_radar_dynamic_update(para_radar, INTERVAL_HIGH);
					parameter_app_update(para, FRAME_RATE_LOW);
				}
				handle_2D->rate_change_flag = 1;
				handle_2D->no_presence_counter = 0;
				return;
			}
		}
		return;
	}
	
	/* Filter spurious peaks from corrupting true ingress event peak count*/
	if (handle_2D->status == 0 && distance < DISTANCE_THRESHOLD)
	{
		return;
	}
		
	/*reset the none presence counter when there is valid measurement*/
	handle_2D->no_presence_counter = 0;

	/*when state is on, continue with on */
	if (handle_2D->status == 1)
	{
		return;
	}

	if (handle_2D->frame_counter == 0)
	{
		handle_2D->frame_counter = 1;
	}

	/*increment the score to confirm the ingress*/
	handle_2D->counter_score += 1;

	/*validate the ingress event*/
	if (handle_2D->frame_counter > (para->turnOn_delay) + 1)    // Wait at least 12 frames.	
	{
		// Decide within 16 frames.
		// If at least 8 of (12 to 16) frames >= threshold, mark it as true ingress event (At least 50%).
		// Latency trade-off to reduce FPR.
		if ((handle_2D->frame_counter <= (para->trigger_outdate_counter) + 1) && (handle_2D->counter_score >= INGRESS_SCORE_THRESHOLD))
		{
			handle_2D->status = 1;
		}
		handle_2D->counter_score = 0;
		handle_2D->frame_counter = 0;
	}

	return;
}

#endif

