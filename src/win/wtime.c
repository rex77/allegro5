/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Windows time module.
 *
 *      By Peter Wang.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro5/allegro5.h"
#include "allegro5/platform/aintwin.h"

#ifndef SCAN_DEPEND
   #include <mmsystem.h>
#endif

#define LARGE_INTEGER_TO_INT64(li) (((int64_t)li.HighPart << 32) | \
	(int64_t)li.LowPart)

static int64_t high_res_timer_freq;
static int64_t _al_win_prev_time;
static double _al_win_total_time;

static double (*real_current_time_func)(void);

static _AL_MUTEX time_mutex = _AL_MUTEX_UNINITED;

static double low_res_current_time(void)
{
    _al_mutex_lock(&time_mutex);
   
   int64_t cur_time = (int64_t) timeGetTime();
   double ellapsed_time = (double) (cur_time - _al_win_prev_time) / 1000;

   if (cur_time < _al_win_prev_time) {
       ellapsed_time += 4294967.295;
   }

   _al_win_total_time += ellapsed_time;
   _al_win_prev_time = cur_time;

   _al_mutex_unlock(&time_mutex);

   return _al_win_total_time;
}


static double high_res_current_time(void)
{
   _al_mutex_lock(&time_mutex);

   LARGE_INTEGER count;
   QueryPerformanceCounter(&count);

   int64_t cur_time = LARGE_INTEGER_TO_INT64(count);
   double ellapsed_time = (double)(cur_time - _al_win_prev_time) / (double)high_res_timer_freq;

   _al_win_total_time += ellapsed_time;
   _al_win_prev_time = cur_time;

   _al_mutex_unlock(&time_mutex);

   return _al_win_total_time;
}


double al_current_time(void)
{
	return (*real_current_time_func)();
}


void _al_win_init_time(void)
{
   LARGE_INTEGER tmp_freq;
   _al_win_total_time = 0;
   QueryPerformanceFrequency(&tmp_freq);

   high_res_timer_freq = LARGE_INTEGER_TO_INT64(tmp_freq);

   if (high_res_timer_freq == 0) {
      real_current_time_func = low_res_current_time;
      _al_win_prev_time = (int64_t) timeGetTime();
   }
   else {
      LARGE_INTEGER count;
      real_current_time_func = high_res_current_time;
      QueryPerformanceCounter(&count);
      _al_win_prev_time = LARGE_INTEGER_TO_INT64(count);
   }
}





/* al_rest:
 *  Rests the specified amount of milliseconds.
 */
void al_rest(double seconds)
{
   ASSERT(seconds >= 0);

   Sleep((DWORD)(seconds * 1000.0));
}
