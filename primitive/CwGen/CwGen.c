/*******************************************************************************
 * 							Primitive CwGen
 * *****************************************************************************
 * 	Filename:		CwGen.c
 * 	Platform:		ubuntu 16.04 64 bit
 * 	Author:			Copyright (C) Selmeczi János, original version
 *******************************************************************************
 *							Licensing
 *******************************************************************************
 *  This file is part of sdrflow.
 *
 *  Sdrflow is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Sdrflow is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 *							Revision history
 *******************************************************************************
 *	Author				Date		Comment
 *******************************************************************************
 *	Selmeczi János		23-04-2018	Original version
 *
 ******************************************************************************/

#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include 	<math.h>
#include	"../../include/primitive_interface.h"
#include	"CwGen.h"

#define SAMPLE_RATE			48000
#define	WPM					20

#define	STATE_START			0
#define	STATE_UP			1
#define	STATE_DOWN			2
#define	STATE_DOT			3
#define	STATE_DASH			4
#define	STATE_SPACE			5
#define	STATE_CHSPACE		6
#define	STATE_WSPACE		7
#define	STATE_END			8

#define	STATE_COUNT			26
#define CODE_COUNT			42

#define	UP_DOWN_TIME_MS		20.0


static const int	state_table[CODE_COUNT][STATE_COUNT] =
{//    1       2       3       4       5       6 
	{1,4,2,5,1,4,2,5,1,4,2,5,1,4,2,5,1,4,2,5,6,8,0,0,0,0}, // 0 - 0
	{1,3,2,5,1,4,2,5,1,4,2,5,1,4,2,5,1,4,2,5,6,8,0,0,0,0}, // 1 - 1
	{1,3,2,5,1,3,2,5,1,4,2,5,1,4,2,5,1,4,2,5,6,8,0,0,0,0}, // 2 - 2
	{1,3,2,5,1,3,2,5,1,3,2,5,1,4,2,5,1,4,2,5,6,8,0,0,0,0}, // 3 - 3
	{1,3,2,5,1,3,2,5,1,3,2,5,1,3,2,5,1,4,2,5,6,8,0,0,0,0}, // 4 - 4
	{1,3,2,5,1,3,2,5,1,3,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0}, // 5 - 5
	{1,4,2,5,1,3,2,5,1,3,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0}, // 6 - 6
	{1,4,2,5,1,4,2,5,1,3,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0}, // 7 - 7
	{1,4,2,5,1,4,2,5,1,4,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0}, // 8 - 8
	{1,4,2,5,1,4,2,5,1,4,2,5,1,4,2,5,1,3,2,5,6,8,0,0,0,0}, // 9 - 9
	{1,3,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // a - 10
	{1,4,2,5,1,3,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0}, // b - 11
	{1,4,2,5,1,3,2,5,1,4,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0}, // c - 12
	{1,4,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0}, // d - 13
	{1,3,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // e - 14
	{1,3,2,5,1,3,2,5,1,4,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0}, // f - 15
	{1,4,2,5,1,4,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0}, // g - 16
	{1,3,2,5,1,3,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0}, // h - 17
	{1,3,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // i - 18
	{1,3,2,5,1,4,2,5,1,4,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0}, // j - 19
	{1,4,2,5,1,3,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0}, // k - 20
	{1,3,2,5,1,4,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0}, // l - 21
	{1,4,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // m - 22
	{1,4,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // n - 23
	{1,4,2,5,1,4,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0}, // o - 24
	{1,3,2,5,1,4,2,5,1,4,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0}, // p - 25
	{1,4,2,5,1,4,2,5,1,3,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0}, // q - 26
	{1,3,2,5,1,4,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0}, // r - 27
	{1,3,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0}, // s - 28
	{1,4,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // t - 29
	{1,3,2,5,1,3,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0}, // u - 30
	{1,3,2,5,1,3,2,5,1,3,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0}, // v - 31
	{1,3,2,5,1,4,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0,0,0,0,0}, // w - 32
	{1,4,2,5,1,3,2,5,1,3,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0}, // x - 33
	{1,4,2,5,1,3,2,5,1,4,2,5,1,4,2,5,6,8,0,0,0,0,0,0,0,0}, // y - 34
	{1,4,2,5,1,4,2,5,1,3,2,5,1,3,2,5,6,8,0,0,0,0,0,0,0,0}, // z - 35
	{7,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // space - 36
	{1,3,2,5,1,4,2,5,1,3,2,5,1,4,2,5,1,3,2,5,6,8,0,0,0,0}, // + - 37
	{1,4,2,5,1,3,2,5,1,3,2,5,1,3,2,5,1,4,2,5,6,8,0,0,0,0}, // = - 38
	{1,3,2,5,1,3,2,5,1,4,2,5,1,4,2,5,1,3,2,5,1,3,2,5,6,8}, // ? - 39
	{1,4,2,5,1,3,2,5,1,3,2,5,1,4,2,5,1,3,2,5,6,8,0,0,0,0}, // / - 40
	{1,4,2,5,1,4,2,5,1,3,2,5,1,3,2,5,1,4,2,5,1,4,2,5,6,8}  // , - 41
};

primitive_catalog_t	CwGen_catalog =
{
	.name =			"CwGen",
	.self_size =	sizeof(cwgen_self_t),
	.init =			&cwgen_init,
	.fire =			&cwgen_fire,
	.cleanup =		&cwgen_cleanup,
	.load =			&cwgen_load,
	.delete =		&cwgen_delete
};

int	cwgen_load(void *context)
{
	return 0;
}

int	cwgen_init(void *context)
{
	cwgen_context_t	*ctx = (cwgen_context_t *) context;	
	cwgen_self_t	*self = ctx->self;
	double			sample_rate = (double) SAMPLE_RATE;

	self->state	= STATE_START;
	self->text_idx = 0;
	self->state_idx = 0;
	self->sample_idx = 0;

	self->up_down_length = (int) round(UP_DOWN_TIME_MS * sample_rate / 1000);
	self->dot_length = (int) round(1.2 * sample_rate / WPM) - self->up_down_length;
	self->dash_length = (int) round(3.6 * sample_rate / WPM) - self->up_down_length;
	self->space_length = self->dot_length;
	self->char_space_length = self->dash_length;
	self->word_space_length = (int) round(4.8 * sample_rate / WPM);

	self->phase = - M_PI / 2.0;
	self->phase_increment = M_PI / ((double) self->up_down_length);

	self->state = 0;

	return 0;
}

int	cwgen_fire(void *context)
{
#ifndef _BENCHMARK_
	cwgen_context_t	*ctx = (cwgen_context_t *) context;	
	cwgen_self_t	*self = ctx->self;
	char			ch;
	int				out_idx;

	out_idx = 0;

	while (out_idx < VECTOR_SIZE)
	{
		switch (self->state)
		{
			case	STATE_START:
				self->state_idx = 0;
				ch = ctx->text[self->text_idx];
				if ((ch >= '0') && (ch <= '9')) self->current_code = ch - '0';
				else if ((ch >= 'a') && (ch <= 'z')) self->current_code = ch - 'a' + 10;
				else if ((ch >= 'A') && (ch <= 'Z')) self->current_code = ch - 'A' + 10;
				else if (ch == ' ') self->current_code = 36;
				else if (ch == '+') self->current_code = 37;
				else if (ch == '=') self->current_code = 38;
				else if (ch == '?') self->current_code = 39;
				else if (ch == '/') self->current_code = 40;
				else if (ch == ',') self->current_code = 41;
				else self->current_code = 36;
				self->state = state_table[self->current_code][self->state_idx];
				break;
			case	STATE_UP:
				if (self->sample_idx == 0) self->phase = - M_PI / 2.0;
				ctx->out_re[out_idx] = 0.5 * (sin(self->phase) + 1.0);
				ctx->out_im[out_idx++] = 0.0;
				self->phase += self->phase_increment;
				self->sample_idx++;
				if (self->sample_idx >= self->up_down_length)
				{
					self->sample_idx = 0;
					self->state_idx++;
					self->state = state_table[self->current_code][self->state_idx];
				}
				break;
			case	STATE_DOWN:
				if (self->sample_idx == 0) self->phase = M_PI / 2.0;
				ctx->out_re[out_idx] = 0.5 * (sin(self->phase) + 1.0);
				ctx->out_im[out_idx++] = 0.0;
				self->phase += self->phase_increment;
				self->sample_idx++;
				if (self->sample_idx >= self->up_down_length)
				{
					self->sample_idx = 0;
					self->state_idx++;
					self->state = state_table[self->current_code][self->state_idx];
				}
				break;
			case	STATE_DOT:
				ctx->out_re[out_idx] = 1.0;
				ctx->out_im[out_idx++] = 0.0;
				self->sample_idx++;
				if (self->sample_idx >= self->dot_length)
				{
					self->sample_idx = 0;
					self->state_idx++;
					self->state = state_table[self->current_code][self->state_idx];
				}
				break;
			case	STATE_DASH:
				ctx->out_re[out_idx] = 1.0;
				ctx->out_im[out_idx++] = 0.0;
				self->sample_idx++;
				if (self->sample_idx >= self->dash_length)
				{
					self->sample_idx = 0;
					self->state_idx++;
					self->state = state_table[self->current_code][self->state_idx];
				}
				break;
			case	STATE_SPACE:
				ctx->out_re[out_idx] = 0.0;
				ctx->out_im[out_idx++] = 0.0;
				self->sample_idx++;
				if (self->sample_idx >= self->space_length)
				{
					self->sample_idx = 0;
					self->state_idx++;
					self->state = state_table[self->current_code][self->state_idx];
				}
				break;
			case	STATE_CHSPACE:
				ctx->out_re[out_idx] = 0.0;
				ctx->out_im[out_idx++] = 0.0;
				self->sample_idx++;
				if (self->sample_idx >= self->char_space_length)
				{
					self->sample_idx = 0;
					self->state_idx++;
					self->state = state_table[self->current_code][self->state_idx];
				}
				break;
			case	STATE_WSPACE:
				ctx->out_re[out_idx] = 0.0;
				ctx->out_im[out_idx++] = 0.0;
				self->sample_idx++;
				if (self->sample_idx >= self->word_space_length)
				{
					self->sample_idx = 0;
					self->state_idx++;
					self->state = state_table[self->current_code][self->state_idx];
				}
				break;
			case	STATE_END:
				self->state_idx = 0;
				self->sample_idx = 0;
				if ((self->text_idx != 0) || (ctx->text[0] != 0))
				{
					self->text_idx++;
					if (ctx->text[self->text_idx] == 0) self->text_idx = 0;
				}
				self->state = STATE_START;
				break;
		}
	}
#endif
	return 0;
}

int	cwgen_cleanup(void *context)
{
	return 0;
}

int	cwgen_delete(void *context)
{
	return 0;
}

