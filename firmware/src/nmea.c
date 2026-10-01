// MIT License
//
// Copyright (c) 2026 Schwarze Lanzenreiter
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdlib.h>
#include <string.h>

#include "nmea.h"

#define KNOT_TO_MPS (1852.0 / 3600.0)

void nmea_init(nmea_fix_t *fix)
{
	memset(fix, 0, sizeof(*fix));
}

void nmea_reader_init(nmea_reader_t *reader)
{
	reader->len = 0;
	reader->overrun = 0;
}

const char *nmea_reader_push(nmea_reader_t *reader, char c)
{
	// a '$' always starts a new sentence. resyncing on it means a burst of line noise costs one
	// sentence rather than desynchronising the reader for good
	if (c == '$') {
		reader->len = 0;
		reader->overrun = 0;
	}

	if (c == '\r' || c == '\n') {
		int overrun = reader->overrun;
		size_t len = reader->len;

		reader->len = 0;
		reader->overrun = 0;

		if (overrun || len == 0) {
			return NULL;
		}

		reader->buf[len] = '\0';
		return reader->buf;
	}

	if (reader->len >= NMEA_MAX_SENTENCE) {
		reader->overrun = 1;
		return NULL;
	}

	reader->buf[reader->len++] = c;
	return NULL;
}

static int hex_value(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

int nmea_checksum_ok(const char *sentence)
{
	unsigned char sum = 0;
	size_t i;
	int hi, lo;

	if (sentence[0] != '$') {
		return 0;
	}

	for (i = 1; sentence[i] != '\0' && sentence[i] != '*'; i++) {
		sum ^= (unsigned char)sentence[i];
	}

	// no '*' means the module was configured without checksums, or the line was cut short. treat
	// it as broken either way, a half sentence decodes into a plausible looking wrong position
	if (sentence[i] != '*') {
		return 0;
	}

	hi = hex_value(sentence[i + 1]);
	lo = (hi < 0) ? -1 : hex_value(sentence[i + 2]);

	if (hi < 0 || lo < 0) {
		return 0;
	}

	return sum == (unsigned char)((hi << 4) | lo);
}

// copies field number index (0 is the sentence name) into out. returns its length, or -1 if the
// sentence does not have that many fields
static int nmea_field(const char *sentence, int index, char *out, size_t outsz)
{
	const char *p = sentence;
	int field = 0;
	size_t len = 0;

	if (*p == '$') {
		p++;
	}

	while (field < index) {
		while (*p != ',' && *p != '*' && *p != '\0') {
			p++;
		}

		if (*p != ',') {
			return -1;
		}

		p++;
		field++;
	}

	// bail out the moment it does not fit, rather than counting the whole field first and checking
	// afterwards: the length of a field comes from the wire, and a bound that is only checked
	// after the fact is one unsigned wrap away from not being a bound at all
	while (*p != ',' && *p != '*' && *p != '\0') {
		if (len + 1 >= outsz) {
			return -1;	// field longer than the caller's buffer. malformed
		}

		out[len++] = *p++;
	}

	out[len] = '\0';
	return (int)len;
}

// "3540.8724" + 'N' -> 35.681206. the degrees part is everything left of the two minutes digits,
// which is why this cannot just be strtod()
static int parse_degrees(const char *value, const char *hemi, double *out)
{
	const char *dot;
	int deg_digits;
	char degbuf[8];
	double degrees, minutes;

	if (value[0] == '\0' || hemi[0] == '\0') {
		return 0;
	}

	dot = strchr(value, '.');
	deg_digits = (dot != NULL) ? (int)(dot - value) - 2 : (int)strlen(value) - 2;

	if (deg_digits < 1 || deg_digits > 3) {
		return 0;
	}

	memcpy(degbuf, value, (size_t)deg_digits);
	degbuf[deg_digits] = '\0';

	degrees = strtod(degbuf, NULL);
	minutes = strtod(value + deg_digits, NULL);

	if (minutes < 0.0 || minutes >= 60.0) {
		return 0;
	}

	*out = degrees + minutes / 60.0;

	if (hemi[0] == 'S' || hemi[0] == 'W') {
		*out = -*out;
	} else if (hemi[0] != 'N' && hemi[0] != 'E') {
		return 0;
	}

	return 1;
}

// days since 1970-01-01 for a proleptic gregorian date. Howard Hinnant's days_from_civil
static int64_t days_from_civil(int y, int m, int d)
{
	int64_t era, yoe, doy, doe;

	y -= (m <= 2);
	era = ((y >= 0) ? y : y - 399) / 400;
	yoe = y - era * 400;						// [0, 399]
	doy = (153 * (m + ((m > 2) ? -3 : 9)) + 2) / 5 + d - 1;	// [0, 365]
	doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;		// [0, 146096]

	return era * 146097 + doe - 719468;
}

int64_t nmea_to_unix(int year, int month, int day, int hour, int min, int sec)
{
	return days_from_civil(year, month, day) * 86400LL + hour * 3600LL + min * 60LL + sec;
}

// "hhmmss" or "hhmmss.sss". the fractional part is dropped, the log's own timestamps are what
// carries sub second resolution
static int parse_time(const char *value, int *hour, int *min, int *sec)
{
	if (strlen(value) < 6) {
		return 0;
	}

	*hour = (value[0] - '0') * 10 + (value[1] - '0');
	*min  = (value[2] - '0') * 10 + (value[3] - '0');
	*sec  = (value[4] - '0') * 10 + (value[5] - '0');

	return (*hour >= 0 && *hour < 24 && *min >= 0 && *min < 60 && *sec >= 0 && *sec <= 60);
}

// "ddmmyy". the two digit year is 20xx. these modules were not made before 2000
static int parse_date(const char *value, int *year, int *month, int *day)
{
	if (strlen(value) < 6) {
		return 0;
	}

	*day   = (value[0] - '0') * 10 + (value[1] - '0');
	*month = (value[2] - '0') * 10 + (value[3] - '0');
	*year  = 2000 + (value[4] - '0') * 10 + (value[5] - '0');

	return (*day >= 1 && *day <= 31 && *month >= 1 && *month <= 12);
}

static int parse_gga(nmea_fix_t *fix, const char *sentence)
{
	char lat[16], ns[4], lon[16], ew[4], quality[8], alt[16], aux[16];
	double latitude, longitude;

	if (nmea_field(sentence, 2, lat, sizeof(lat)) < 0 ||
	    nmea_field(sentence, 3, ns, sizeof(ns)) < 0 ||
	    nmea_field(sentence, 4, lon, sizeof(lon)) < 0 ||
	    nmea_field(sentence, 5, ew, sizeof(ew)) < 0 ||
	    nmea_field(sentence, 6, quality, sizeof(quality)) < 0) {
		return NMEA_RESULT_INVALID;
	}

	fix->quality = (quality[0] == '\0') ? 0 : (int)strtol(quality, NULL, 10);

	// quality 0 is "no fix". 1 is GPS, 2 is DGPS, and the rest (RTK, estimated) are all better
	// positioned than that, so anything non zero counts. this mirrors the pi version accepting
	// both STATUS_FIX and STATUS_DGPS_FIX
	if (fix->quality == 0) {
		fix->has_fix = 0;
		fix->has_altitude = 0;

		// the receiver still reports what it can see while searching, and that is the number
		// worth watching during a long cold start
		if (nmea_field(sentence, 7, aux, sizeof(aux)) > 0) {
			fix->sats_used = (int)strtol(aux, NULL, 10);
		}

		return NMEA_RESULT_GGA;
	}

	if (!parse_degrees(lat, ns, &latitude) || !parse_degrees(lon, ew, &longitude)) {
		fix->has_fix = 0;
		return NMEA_RESULT_GGA;
	}

	fix->latitude = latitude;
	fix->longitude = longitude;
	fix->has_fix = 1;

	// satellite count and horizontal dilution, for the diagnostics only. a receiver that is using
	// three satellites with an HDOP of 8 is about to lose the fix again, and that is worth seeing
	// in the log rather than guessing at
	if (nmea_field(sentence, 7, aux, sizeof(aux)) > 0) {
		fix->sats_used = (int)strtol(aux, NULL, 10);
	}

	fix->hdop = (nmea_field(sentence, 8, aux, sizeof(aux)) > 0) ? strtod(aux, NULL) : 0.0;

	// field 9 is the MSL altitude. it is empty while the fix is 2D only, which is the case the
	// pi version saw as a NaN altitude from gpsd and answered by skipping the 0x7FE frame
	if (nmea_field(sentence, 9, alt, sizeof(alt)) > 0) {
		fix->altitude = strtod(alt, NULL);
		fix->has_altitude = 1;
	} else {
		fix->has_altitude = 0;
	}

	return NMEA_RESULT_GGA;
}

static int parse_rmc(nmea_fix_t *fix, const char *sentence)
{
	char status[4], lat[16], ns[4], lon[16], ew[4], knots[16], timebuf[16], datebuf[16];
	double latitude, longitude;
	int hour, min, sec, year, month, day;

	if (nmea_field(sentence, 1, timebuf, sizeof(timebuf)) < 0 ||
	    nmea_field(sentence, 2, status, sizeof(status)) < 0 ||
	    nmea_field(sentence, 3, lat, sizeof(lat)) < 0 ||
	    nmea_field(sentence, 4, ns, sizeof(ns)) < 0 ||
	    nmea_field(sentence, 5, lon, sizeof(lon)) < 0 ||
	    nmea_field(sentence, 6, ew, sizeof(ew)) < 0 ||
	    nmea_field(sentence, 7, knots, sizeof(knots)) < 0 ||
	    nmea_field(sentence, 9, datebuf, sizeof(datebuf)) < 0) {
		return NMEA_RESULT_INVALID;
	}

	// the date is valid even in a 'V' sentence on most modules, and it is what names the log file,
	// so take it before looking at the status
	if (parse_time(timebuf, &hour, &min, &sec) && parse_date(datebuf, &year, &month, &day)) {
		fix->unix_time = nmea_to_unix(year, month, day, hour, min, sec);
		fix->has_time = 1;
	}

	if (status[0] != 'A') {
		fix->has_fix = 0;
		fix->has_speed = 0;
		return NMEA_RESULT_RMC;
	}

	if (!parse_degrees(lat, ns, &latitude) || !parse_degrees(lon, ew, &longitude)) {
		fix->has_fix = 0;
		return NMEA_RESULT_RMC;
	}

	fix->latitude = latitude;
	fix->longitude = longitude;
	fix->has_fix = 1;

	if (knots[0] != '\0') {
		fix->speed = strtod(knots, NULL) * KNOT_TO_MPS;
		fix->has_speed = 1;
	} else {
		fix->has_speed = 0;
	}

	return NMEA_RESULT_RMC;
}

// GSA: $..GSA,mode1,mode2,sat1..sat12,pdop,hdop,vdop
// mode2 is the honest answer to "is this thing fixed": 1 none, 2 two dimensional, 3 three
static int parse_gsa(nmea_fix_t *fix, const char *sentence)
{
	char buf[16];

	if (nmea_field(sentence, 2, buf, sizeof(buf)) > 0) {
		fix->fix_mode = (int)strtol(buf, NULL, 10);
	}

	if (nmea_field(sentence, 15, buf, sizeof(buf)) > 0) {
		fix->pdop = strtod(buf, NULL);
	}

	// GSA also carries HDOP. GGA is the primary source, so only fill in when it said nothing
	if (fix->hdop == 0.0 && nmea_field(sentence, 16, buf, sizeof(buf)) > 0) {
		fix->hdop = strtod(buf, NULL);
	}

	if (nmea_field(sentence, 17, buf, sizeof(buf)) > 0) {
		fix->vdop = strtod(buf, NULL);
	}

	return NMEA_RESULT_GSA;
}

// finds the accumulator for this talker, or claims a free slot. returns NULL when all slots are
// taken, which just means that constellation goes uncounted
static nmea_gsv_t *gsv_slot(nmea_fix_t *fix, const char *talker)
{
	int i;

	for (i = 0; i < NMEA_MAX_TALKERS; i++) {
		if (fix->gsv[i].id[0] == talker[0] && fix->gsv[i].id[1] == talker[1]) {
			return &fix->gsv[i];
		}
	}

	for (i = 0; i < NMEA_MAX_TALKERS; i++) {
		if (fix->gsv[i].id[0] == '\0') {
			fix->gsv[i].id[0] = talker[0];
			fix->gsv[i].id[1] = talker[1];
			fix->gsv[i].id[2] = '\0';
			return &fix->gsv[i];
		}
	}

	return NULL;
}

// GSV: $..GSV,total,num,in_view,{prn,elevation,azimuth,snr} x up to 4
//
// a receiver reports every satellite it knows about, with an empty SNR field for the ones it is
// not yet tracking. the gap between "in view" and "tracked" is what a bad antenna or a garage
// roof looks like, so both are counted
static int parse_gsv(nmea_fix_t *fix, const char *sentence)
{
	char buf[16];
	nmea_gsv_t *slot = gsv_slot(fix, sentence + 1);
	int total, num, in_view, group;

	if (slot == NULL) {
		return NMEA_RESULT_GSV;
	}

	if (nmea_field(sentence, 1, buf, sizeof(buf)) <= 0) {
		return NMEA_RESULT_GSV;
	}

	total = (int)strtol(buf, NULL, 10);

	if (nmea_field(sentence, 2, buf, sizeof(buf)) <= 0) {
		return NMEA_RESULT_GSV;
	}

	num = (int)strtol(buf, NULL, 10);

	in_view = (nmea_field(sentence, 3, buf, sizeof(buf)) > 0) ? (int)strtol(buf, NULL, 10) : 0;

	// first sentence of a cycle. start counting again, or a receiver that loses satellites would
	// never show the loss
	if (num <= 1) {
		slot->acc_tracked = 0;
		slot->acc_best_snr = 0;
	}

	for (group = 0; group < 4; group++) {
		int snr_field = 4 + group * 4 + 3;

		if (nmea_field(sentence, snr_field, buf, sizeof(buf)) <= 0) {
			continue;	// no SNR: known from the almanac but not being received
		}

		slot->acc_tracked++;

		if ((int)strtol(buf, NULL, 10) > slot->acc_best_snr) {
			slot->acc_best_snr = (int)strtol(buf, NULL, 10);
		}
	}

	// last sentence of the cycle. publish what was counted
	if (num >= total) {
		slot->in_view = in_view;
		slot->tracked = slot->acc_tracked;
		slot->best_snr = slot->acc_best_snr;
	}

	return NMEA_RESULT_GSV;
}

int nmea_sats_in_view(const nmea_fix_t *fix)
{
	int i, total = 0;

	for (i = 0; i < NMEA_MAX_TALKERS; i++) {
		total += fix->gsv[i].in_view;
	}

	return total;
}

int nmea_sats_tracked(const nmea_fix_t *fix)
{
	int i, total = 0;

	for (i = 0; i < NMEA_MAX_TALKERS; i++) {
		total += fix->gsv[i].tracked;
	}

	return total;
}

int nmea_best_snr(const nmea_fix_t *fix)
{
	int i, best = 0;

	for (i = 0; i < NMEA_MAX_TALKERS; i++) {
		if (fix->gsv[i].best_snr > best) {
			best = fix->gsv[i].best_snr;
		}
	}

	return best;
}

int nmea_parse(nmea_fix_t *fix, const char *sentence)
{
	const char *type;

	if (sentence[0] != '$' || strlen(sentence) < 7) {
		return NMEA_RESULT_INVALID;
	}

	if (!nmea_checksum_ok(sentence)) {
		return NMEA_RESULT_INVALID;
	}

	// skip the two character talker id. GPS only modules say GP, multi constellation ones say GN
	// and may also emit GL/GA/GB sentences of their own, which carry the same fields
	type = sentence + 3;

	if (strncmp(type, "GGA", 3) == 0) {
		return parse_gga(fix, sentence);
	}

	if (strncmp(type, "RMC", 3) == 0) {
		return parse_rmc(fix, sentence);
	}

	if (strncmp(type, "GSA", 3) == 0) {
		return parse_gsa(fix, sentence);
	}

	if (strncmp(type, "GSV", 3) == 0) {
		return parse_gsv(fix, sentence);
	}

	return NMEA_RESULT_IGNORED;
}
