/*
!C****************************************************************************

!File: parser.c
  
!Description: Functions for parsing parameters from the command line or 
 a file.

!Revision History:
 Revision 1.1 2000/12/13
 Sadashiva Devadiga
 Original version.

 Revision 1.2 2001/05/08
 Sadashiva Devadiga
 Added support for gridded input (in addition to swath input).

 Revision 1.3 2002/05/10
 Robert Wolfe
 a. Added separate output SDS name.
 b. Added usage information for parameter file.

 Revision 1.5 2002/06/13
 Gail Schmidt
 Added lower right corner definition. Instead of entering the upper left
 in output projection space and the number of lines and samples, the
 user now enters the upper left (long lat) and lower right (long lat).
 The output UL projection coords and number of lines and samples in
 output space are then calculated. Long and Lat are input in decimal
 degrees.

 Revision 1.5 2002/12/02
 Gail Schmidt
 Added support for INT8 data types.

 Revision 2.0 2003/Nov-Dec
 Gail Schmidt
 Modified the software to process all the SDSs, if an SDS was not specified
   in the parameter file. And, support multiple SDSs for input by the user.
 Also, GRIDs will not be used by MRTSwath, so don't allow GRID processing.
 Allow a '#' as the first character in the parameter file to denote a
   commented line, which won't be processed.
 Support output to raw binary.
 Added support for multiple pixel sizes. This also means multiple
   lines and samples.
 Allow the user to specify the UL/LR corner in line/sample units.

!Team Unique Header:
  This software was developed by the MODIS Land Science Team Support 
  Group for the Laboratory for Terrestrial Physics (Code 922) at the 
  National Aeronautics and Space Administration, Goddard Space Flight 
  Center, under NASA Task 92-012-00.

 ! References and Credits:
  ! MODIS Science Team Member:
      Christopher O. Justice
      MODIS Land Science Team           University of Maryland
      justice@hermes.geog.umd.edu       Dept. of Geography
      phone: 301-405-1600               1113 LeFrak Hall
                                        College Park, MD, 20742

  ! Developers:
      Robert E. Wolfe (Code 922)
      MODIS Land Team Support Group     Raytheon ITSS
      robert.e.wolfe.1@gsfc.nasa.gov    4400 Forbes Blvd.
      phone: 301-614-5508               Lanham, MD 20770  

      Sadashiva Devadiga (Code 922)
      MODIS Land Team Support Group     SSAI
      devadiga@ltpmail.gsfc.nasa.gov    5900 Princess Garden Pkway, #300
      phone: 301-614-5549               Lanham, MD 20706

      Gail Schmidt
      SAIC / USGS EROS Data Center
      Rapid City, SD 57702
      gschmidt@usgs.gov
      Added support for GEOTIFF and/or HDF output formats. This is supported
      by the -off cmd-line parameter or the OUTPUT_FILE_FORMAT argument
      in the parameter file. Valid choices are HDF_FMT, GEOTIFF_FMT,
      or BOTH (for both HDF and GeoTiff).
      Added support for raw binary format (RB_FMT). Valid choices are
      HDF_FMT, GEOTIFF_FMT, RB_FMT, or BOTH (for both HDF and GeoTiff).
  
 ! Design Notes:
   1. The following public functions parse the user parameters:

        ReadCmdLine - Parses the command line and gets the user parameters.
	NeedHelp - Determines if the user has asked for help and, if so,
	  prints help information.

   2. The following internal functions are also used to parse the user 
      parameters:

	ReadParamFile - parses a parameter file and gets the user 
          parameters.
	GetProjNum - converts a projection id string to projection 
          number.
	IsArgID - checks a complete input option string for a specific
          option id.
	GetArgValArray - parses a complete input option string for a 
          set of option values.
	GetArgVal - parses a complete input option string for a single 
          option value.
	Calloc2D - allocate memory for a two dimensional array.
	Free2D - frees memory for a two dimensional array.
	charpos - returns the location of the first occurrence of a
          character in a string.
	strpos - returns the location of the first occurrence of a
          substring in a string.
	strtrim - removes the leading and trailing blanks from a string.
	strupper - converts a string to upper case.
	strmid - extracts a substring from a string.
	update_sds_info - initializes the SDS name, rank and dimension
          index information.
	get_line - reads a line from a file.

!END****************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>        

#include "parser.h"
#include "usage.h"
#include "param.h"
#include "geoloc.h"
#include "myproj_const.h"
#include "bool.h"
#include "myerror.h"
#include "myisoc.h"


/* Constants */
#define MAX_OPTION_VAL_LEN (255)
/* #define FILE_BUFSIZ (64*1024) */
#define LINE_BUFSIZ (5*1024)


/* Internal Prototypes */

bool ReadParamFile(FILE *file, Param_t *this);
int GetProjNum(char *proj_str);
bool IsArgID(const char *arg_str, char *arg_id);
void GetArgValArray(const char *arg_str, char **arg_val, int *arg_cnt);
char *GetArgVal(const char *arg_str);
void **Calloc2D(size_t nobj1, size_t nobj2, size_t size);
void Free2D(void **p1);
int charpos(const char *s, char c, int p);
int strpos(const char *s1, const char *s2, int p);
void strtrim(char *s);
void strupper(char *);
void strmid(const char *s1, int p1, int cnt, char *s2);
bool update_sds_info(int sdsnum, Param_t *this);
int get_line(FILE *fp, char *s);
bool parse_sds_name(Param_t *this, char *sds_string);
bool strip_blanks(char *instr);
int CleanupLine(char *line);
char *removeDoubleQuotes( char *str );

/* Functions */

bool ReadCmdLine(int argc, const char **argv, Param_t *this)
/* 
!C******************************************************************************

!Description: 'ReadCmdLine' parses the command line and gets the user 
 parameters.
 
!Input Parameters:
 argc           number of command line arguments
 argv           command line argument list

!Output Parameters:
 this           'param' data structure; all fields may be modified
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. memory allocation is not successful
       b. an argument value is not given
       c. an argument value is invalid:
            output space projection parameter value, output space pixel size,
	    output space upper left corner, output space image size,
	    output space sphere number, output space zone number,
	    input space projection parameter value, input space pixel size,
	    input space upper left corner, input space image size,
	    input space sphere number, input space zone number,
	    output data type
       d. an invalid option is given.
   2. Errors of type 'a' are handled with the 'LOG_RETURN_ERROR' macro and 
      the others are handled by writing the error messages to 'stderr'.
   3. A warning is written to 'stderr' if all of the projection parameters 
      are not given.

!END****************************************************************************
*/
{
  int iarg, ip, n;
  char *tmp, **tmp_arr;
  bool st, loc_st;
  char *error_string;
  FILE *file;
  char *input_sds_string;
  char msg[M_MSG_LEN+1];

  tmp_arr = (char **)Calloc2D(MAX_NUM_PARAM, MAX_OPTION_VAL_LEN, sizeof(char));
  if (tmp_arr == (char **)NULL) 
    LOG_RETURN_ERROR("cannot allocate memory for tmp_arr", "ReadCmdLine",false);

  /* Get the parameters from command line */

  st = true;
  error_string = (char *)NULL;

  for (iarg = 1; iarg < argc; iarg++) {

    if (error_string != (char *)NULL) {
      sprintf(msg, "resamp: %s", error_string);
      LogInfomsg(msg);
      st = false;
      error_string = (char *)NULL;
    }

    if (IsArgID(argv[iarg], "-if")) {
      this->input_file_name = GetArgVal(argv[iarg]);
      if (this->input_file_name == (char *)NULL) {
        error_string = "can't get argument value (-if)";
      }
    }

    else if (IsArgID(argv[iarg], "-of")) {
      this->output_file_name = GetArgVal(argv[iarg]);
      if (this->output_file_name == (char *)NULL) {
        error_string = "can't get argument value (-of)";
      }
    }

    else if (IsArgID(argv[iarg], "-gf")) {
      this->geoloc_file_name = GetArgVal(argv[iarg]);
      if (this->geoloc_file_name == (char *)NULL) {
        error_string = "can't get argument value (-gf)";
      }
    }

    else if (IsArgID(argv[iarg], "-sds")) {
      input_sds_string = GetArgVal(argv[iarg]);
      if (input_sds_string == (char *)NULL) {
        this->num_input_sds = 0;
	continue;
      }
      parse_sds_name(this, input_sds_string);
    }

    else if (IsArgID(argv[iarg], "-kk")) {
      tmp = GetArgVal(argv[iarg]);
      if (tmp == (char *)NULL) {
        error_string = "can't get argument value (-kk)";
	continue;
      }
      strupper(tmp);
      if (strcmp(tmp, "NN") == 0) {
        this->kernel_type = NN;
	free(tmp);
      } else if (strcmp(tmp, "BI") == 0) {
        this->kernel_type = BL;
	free(tmp);
      } else if (strcmp(tmp, "CC") == 0) {
        this->kernel_type = CC;
	free(tmp);
      }
    }

    else if(IsArgID(argv[iarg],"-off")) {
      tmp = GetArgVal(argv[iarg]);
      if(tmp == (char *)NULL) {
        error_string = "can't get argument value (-off)";
	continue;
      }
      strupper(tmp);
      if(strcmp(tmp,"HDF_FMT") == 0) {
        this->output_file_format = HDF_FMT;
        free(tmp);
      } else if (strcmp(tmp,"GEOTIFF_FMT") == 0) {
        this->output_file_format = GEOTIFF_FMT;
        free(tmp);
      } else if (strcmp(tmp,"RB_FMT") == 0) {
        this->output_file_format = RB_FMT;
        free(tmp);
      } else if (strcmp(tmp,"BOTH") == 0) {
        this->output_file_format = BOTH;
        free(tmp);
      }
    }

    else if (IsArgID(argv[iarg], "-oproj")) {
      tmp = GetArgVal(argv[iarg]);
      if (tmp == (char *)NULL) {
        error_string = "can't get argument value (-oproj)";
	continue;
      }
      this->output_space_def.proj_num = GetProjNum(tmp);     
      free(tmp);
    }

    else if (IsArgID(argv[iarg], "-oprm")) {
      GetArgValArray(argv[iarg], tmp_arr, &n);
      if (n != NPROJ_PARAM)
      {
        sprintf(msg, "resamp: (warning) only first %d of %d elements in " 
                     "output space projection parameter (-oprm) are input. "
                     "Default value (0.0) used for the remaining "
                     "elements. \n", n, (int)NPROJ_PARAM);
        LogInfomsg(msg);
      }

      loc_st = true;
      for (ip = 0; ip < n; ip++) {
	if (sscanf(tmp_arr[ip], "%lg", 
	           &this->output_space_def.proj_param[ip]) != 1) {
          sprintf(msg, "resamp: invalid output space projection parameter "
	               "value (-oprm[%d]=%s).\n", ip, tmp_arr[ip]);
          LogInfomsg(msg);
          loc_st = false;
        }
      }
      if (!loc_st) st = false;
    }

    else if (IsArgID(argv[iarg], "-opsz")) {
      GetArgValArray(argv[iarg], tmp_arr, &n);
      if (n > MAX_SDS_DIMS) {
        sprintf(msg, "resamp: (warning) only the first %d pixel sizes "
                     "will be used (out of %d supplied by the user), "
                     "since %d is the maximum number of SDSs allowed "
                     "in an HDF file. \n", MAX_SDS_DIMS, n, MAX_SDS_DIMS);
	LogInfomsg(msg);
        n = MAX_SDS_DIMS;
      }
      loc_st = true;
      for (ip = 0; ip < n; ip++) {
        if (sscanf(tmp_arr[ip], "%lg",
                   &this->output_pixel_size[ip]) != 1) {
          sprintf(msg, "resamp: invalid output space pixel size "
                       "value (-opsz[%d]=%s).\n", ip, tmp_arr[ip]);
	  LogInfomsg(msg);
          loc_st = false;
        }

        if (this->output_pixel_size[ip] <= 0.0) {
          sprintf(msg, "resamp: output space pixel size out of valid "
	    "range (-opsz[%d]=%s).\n", ip, tmp_arr[ip]);
          LogInfomsg(msg);
          loc_st = false;
        }
      }
      if (!loc_st) st = false;
    }

    else if (IsArgID(argv[iarg], "-oul")) {
      GetArgValArray(argv[iarg], tmp_arr, &n);
      if (n != 2) {
        sprintf(msg, "resamp: invalid output space upper left "
                "corner (%s).\n", argv[iarg]);
	LogInfomsg(msg);
        st = false;
      } else {
        if ((sscanf(tmp_arr[0], "%lg",
                    &this->output_space_def.ul_corner.x) != 1)  ||
            (sscanf(tmp_arr[1], "%lg",
                    &this->output_space_def.ul_corner.y) != 1)) {
          sprintf(msg, "resamp: invalid output space upper left "
                  "corner (%s).\n", argv[iarg]);
	  LogInfomsg(msg);
          st = false;
        } else
          this->output_space_def.ul_corner_set = true;
      }
    }

    else if (IsArgID(argv[iarg], "-olr")) {
      GetArgValArray(argv[iarg], tmp_arr, &n);
      if (n != 2) {
        sprintf(msg, "resamp: invalid output space lower right "
                "corner (%s).\n", argv[iarg]);
	LogInfomsg(msg);
        st = false;
      } else {
        if ((sscanf(tmp_arr[0], "%lg", 
                    &this->output_space_def.lr_corner.x) != 1)  ||
            (sscanf(tmp_arr[1], "%lg",
                    &this->output_space_def.lr_corner.y) != 1)) {
          sprintf(msg, "resamp: invalid output space lower right "
                  "corner (%s).\n", argv[iarg]);
	  LogInfomsg(msg);
          st = false;
        } else
          this->output_space_def.lr_corner_set = true;
      }
    }

    else if (IsArgID(argv[iarg], "-osst")) {
      tmp = GetArgVal(argv[iarg]);
      if (tmp == (char *)NULL) {
        error_string = "can't get argument value (-osst)";
        continue;
      }
      strupper(tmp);
      if (strcmp(tmp, "LAT_LONG") == 0) {
        this->output_spatial_subset_type = LAT_LONG;
        free(tmp);
      } else if (strcmp(tmp, "PROJ_COORDS") == 0) {
        this->output_spatial_subset_type = PROJ_COORDS;
        free(tmp);
      } else if (strcmp(tmp, "LINE_SAMPLE") == 0) {
        this->output_spatial_subset_type = LINE_SAMPLE;
        free(tmp);
      }
    }

    else if (IsArgID(argv[iarg], "-osp")) {
      tmp = GetArgVal(argv[iarg]);
      if (tmp == (char *)NULL) {	
        error_string = "can't get argument value (-osp)";
	continue;
      }
      if (sscanf(tmp, "%d", &this->output_space_def.sphere) != 1) {
        sprintf(msg, "resamp: invalid output space sphere "
	        "number (%s).\n", argv[iarg]);
	LogInfomsg(msg);
        st = false;
      } else if (this->output_space_def.sphere < 0) {
        sprintf(msg, "resamp: output space sphere number out "
	        "of valid range (-osp=%s).\n", tmp);
	LogInfomsg(msg);
        st = false;
      }
      free(tmp);
    }

    else if (IsArgID(argv[iarg], "-ozn")) {
      tmp = GetArgVal(argv[iarg]);
      if (tmp == (char *)NULL) {
        error_string = "can't get argument value (-ozn)";
	continue;
      }
      if (sscanf(tmp, "%d", &this->output_space_def.zone) != 1) {
        sprintf(msg, "resamp: invalid output space zone "
	        "number (%s).\n", argv[iarg]);
	LogInfomsg(msg);
        st = false;
      } else this->output_space_def.zone_set = true;
      free(tmp);
    }

    else if (IsArgID(argv[iarg], "-iul")) {
      GetArgValArray(argv[iarg], tmp_arr, &n);
      if (n != 2) {
        sprintf(msg, "resamp: invalid input space upper left "
                "corner (%s).\n", argv[iarg]);
	LogInfomsg(msg);
        st = false;
      } else {
        if ((sscanf(tmp_arr[0], "%lg",
                    &this->input_space_def.ul_corner.x) != 1)  ||
            (sscanf(tmp_arr[1], "%lg",
                    &this->input_space_def.ul_corner.y) != 1)) {
          sprintf(msg, "resamp: invalid input space upper left "
                  "corner (%s).\n", argv[iarg]);
	  LogInfomsg(msg);
          st = false;
        } else
          this->input_space_def.ul_corner_set = true;
      }
    }

    else if (IsArgID(argv[iarg], "-oty")) {
      tmp = GetArgVal(argv[iarg]);
      if (tmp == (char *)NULL) {
        error_string = "can't get argument value (-oty)";
	continue;
      }
      strupper(tmp);
      if (strcmp(tmp, "CHAR8") == 0) this->output_data_type = DFNT_CHAR8;
      else if (strcmp(tmp, "UINT8") == 0) this->output_data_type = DFNT_UINT8;
      else if (strcmp(tmp, "INT8") == 0) this->output_data_type = DFNT_INT8;
      else if (strcmp(tmp, "INT16") == 0)this->output_data_type = DFNT_INT16;
      else if (strcmp(tmp, "UINT16") == 0) this->output_data_type = DFNT_UINT16;
      else if (strcmp(tmp, "INT32") == 0) this->output_data_type = DFNT_INT32;
      else if (strcmp(tmp, "UINT32") == 0) this->output_data_type = DFNT_UINT32;
      else {
        sprintf(msg, "resamp: invalid output data type (%s).\n", 
	        argv[iarg]);
	LogInfomsg(msg);
        st = false;
      }
      free(tmp);
    }

    else if (IsArgID(argv[iarg], "-pf")) {
      tmp = GetArgVal(argv[iarg]);
      if (tmp == (char *)NULL) {
        error_string = "can't get argument value (-pf)";
	continue;
      }

      file = fopen(tmp, "r");
      if (file == (FILE *)NULL) {
        error_string = "can't open parameter file (-pf)";
        free(tmp);
        continue;
      }
      free(tmp);

      if (!ReadParamFile(file, this)) {
        error_string = "error reading the parameter file (-pf)";
      }
      fclose(file);
    }

    else {
      sprintf(msg, "resamp: invalid option (%s)\n", argv[iarg]);
      LogInfomsg(msg);
      st = false;
    }

  }

  Free2D((void **)tmp_arr);

  if (error_string != (char *)NULL) {
    sprintf(msg, "resamp: %s.", error_string);
    LogInfomsg(msg);
    st = false;
  }

  return st;
}
                               

bool ReadParamFile(FILE *file, Param_t *this)
/* 
!C******************************************************************************

!Description: 'ReadParamFile' parses a parameter file and gets the user 
 parameters.
 
!Input Parameters:
 file           file handle

!Output Parameters:
 this           'param' data structure; all fields may be modified
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. memory allocation is not successful
       b. an argument value is not given
       c. an argument value is invalid:
            kernel type, 
	    output space projection parameter value, output space pixel size,
	    output space upper left corner, output space image size,
	    output space sphere number, output space zone number,
	    input space projection parameter value, input space pixel size,
	    input space upper left corner, input space image size,
	    input space sphere number, input space zone number,
	    output data type, append value
       d. an invalid option is given.
   2. Errors of type 'a' are handled with the 'LOG_RETURN_ERROR' macro and 
      the others are handled by writing the error messages to 'stderr'.
   3. A warning is written to 'stderr' if all of the projection parameters 
      are not given.

!END****************************************************************************
*/
{
  int p1, p2, len; 
  bool st, loc_st;
  int n, ip;
  int len_arg_id;
  char *arg_val = NULL, **tmp_arr;
  char s[MAX_SDS_STR_LEN], arg_id[MAX_STR_LEN];
  char *error_string;
  char msg[M_MSG_LEN+1];

  printf("debug 1\n");

  tmp_arr = (char **)Calloc2D(MAX_NUM_PARAM, MAX_OPTION_VAL_LEN, sizeof(char));
  if (tmp_arr == (char **)NULL) 
    LOG_RETURN_ERROR("cannot allocate memory for tmp_arr", "ReadParamFile",
                     false);

  printf("debug 2\n");

  st = true;
  error_string = (char *)NULL;

  /* Get the next line .... up to MAX_SDS_STR_LEN characters.  Needs
     to be large since the list of SDSs can be up to 5000 characters. */
  while ( fgets( s, MAX_SDS_STR_LEN, file ) ) {
    /* cleanup the spaces and newlines */
    if ( (len = CleanupLine( s )) == 0 ) {
      continue;
    }

    /* if the first character in the line is a '#' then skip the line */
    if (s[0] == '#')
      continue;

    if ((p1 = charpos(s, '=', 0)) == -1) p1 = len - 1;
    strmid(s, 0, p1, arg_id);
    strtrim(arg_id);
    strupper(arg_id);
    len_arg_id = strlen(arg_id);
    if (len_arg_id == 0) continue;

    p2 = len - p1 - 1;

    if (p2 <= 0) {
      arg_val = (char *)NULL;
    }
    else {
      arg_val = (char *)calloc((p2 + 1), sizeof(char));
      if (arg_val == (char *)NULL) {
        error_string = "unable to allocate memory";
        break;
      }
      strmid(s, (p1 + 1), p2, arg_val);
      strtrim(arg_val);
    }

    if ((strcmp(arg_id, "IF") == 0)  ||
        (strcmp(arg_id, "INPUT_FILENAME") == 0)) {
      if (arg_val == (char *)NULL) {
          error_string = "null input file name";
          break;
      } else {
        this->input_file_name = removeDoubleQuotes(strdup(arg_val));
      }
    }

    else if ((strcmp(arg_id, "OF") == 0)  ||
             (strcmp(arg_id, "OUTPUT_FILENAME") == 0)) {
      if (arg_val == (char *)NULL) {
          error_string = "null output file name";
          break;
      } else {
        this->output_file_name = removeDoubleQuotes(strdup(arg_val));
      }
    }

    else if ((strcmp(arg_id, "GF") == 0)  ||
             (strcmp(arg_id, "GEOLOCATION_FILENAME") == 0)) {
      if (arg_val == (char *)NULL) {
          error_string = "null geolocation file name";
          break;
      } else {
        this->geoloc_file_name = removeDoubleQuotes(strdup(arg_val));
      }
    }

    else if((strcmp(arg_id,"OFF") == 0) || 
            (strcmp(arg_id,"OUTPUT_FILE_FORMAT") == 0)) {
      if (arg_val == (char *)NULL) {
        error_string  = "null output file format value";
        break;
      } else {
        strupper(arg_val);
        if(strcmp(arg_val, "HDF_FMT") == 0)
          this->output_file_format = HDF_FMT;
        else if(strcmp(arg_val, "GEOTIFF_FMT") == 0)
          this->output_file_format = GEOTIFF_FMT;
        else if(strcmp(arg_val, "RB_FMT") == 0)
          this->output_file_format = RB_FMT;
        else if(strcmp(arg_val, "BOTH") == 0)
          this->output_file_format = BOTH;
        else {
          sprintf(msg, "resamp: invalid output file format value (%s).\n", 
	          arg_val);
	  LogInfomsg(msg);
          error_string = "invalid output file format";
          break;
	}
      }       
    }

    else if ((strcmp(arg_id, "SDS") == 0)  ||
             (strcmp(arg_id, "INPUT_SDS_NAME") == 0)) {
      if (arg_val == (char *)NULL) {
          this->num_input_sds = 0;
      } else {
        parse_sds_name(this, arg_val);
      }
    }

    else if (strcmp(arg_id, "KK") == 0) {
      if (arg_val == (char *)NULL) {
          error_string = "null kernel type";
          break;
      } else {
        strupper(arg_val);
        if (strcmp(arg_val, "NN") == 0) {
          this->kernel_type = NN;
        } else if (strcmp(arg_val, "BI") == 0) {
          this->kernel_type = BL;
	} else if (strcmp(arg_val, "CC") == 0) {
	  this->kernel_type = CC;
	}
        else {
          sprintf(msg, "resamp: invalid kernel type value (%s).\n", 
	          arg_val);
	  LogInfomsg(msg);
          error_string = "invalid kernel type";
          break;
	}
      }
    }

    else if ((len_arg_id >= 11)  &&
             (strncmp(arg_id, "KERNEL_TYPE", 11) == 0)) {
      if (arg_val == (char *)NULL) {
        error_string = "null kernel type";
        break;
      } else {
        strupper(arg_val);
        if (strcmp(arg_val, "NN") == 0) this->kernel_type = NN;
        else if (strcmp(arg_val, "BI") == 0) this->kernel_type = BL;
        else if (strcmp(arg_val, "CC") == 0) this->kernel_type = CC;
        else {
          sprintf(msg, "resamp: invalid kernel type value (%s).\n", 
	          arg_val);
	  LogInfomsg(msg);
          error_string = "invalid kernel type";
	  break;
        }
      }
    }

    else if ((strcmp(arg_id, "OPROJ") == 0)  ||
             (strcmp(arg_id, "OUTPUT_PROJECTION_NUMBER") == 0)) {
      if (arg_val == (char *)NULL) {
          error_string = "null projection number";
          break;
      } else {
        this->output_space_def.proj_num = GetProjNum(arg_val);
      }
    }

    else if ((strcmp(arg_id, "OPRM") == 0)  ||
             (strcmp(arg_id, "OUTPUT_PROJECTION_PARAMETER") == 0)) {
      if (arg_val == (char *)NULL) {
        error_string = "null projection parameters";
        break;
      } else {
        sprintf(s, "%s=%s", arg_id, arg_val);
        GetArgValArray(s, tmp_arr, &n);
        if (n != NPROJ_PARAM) {
          sprintf(msg, "resamp: (warning) only first %d of %d elements in " 
	               "output space projection parameter (-prm) are input. "
		       "Default value (0.0) used for the remaining "
		       "elements. \n", n, (int)NPROJ_PARAM);
	       LogInfomsg(msg);
        }
      }
      loc_st = true;
      for (ip = 0; ip < n; ip++) {
	if (sscanf(tmp_arr[ip], "%lg",
            &this->output_space_def.proj_param[ip]) != 1) {
          sprintf(msg, "resamp: invalid output space projection parameter "
	               "value ([%d]=%s).\n", ip, tmp_arr[ip]);
	  LogInfomsg(msg);
          error_string = "invalid projection parameters";
          break;
        }
      }
      if (!loc_st) {
        error_string = "invalid projection parameters";
        break;
      }
    }

    else if ((strcmp(arg_id, "OPSZ") == 0)  ||
             (strcmp(arg_id, "OUTPUT_PIXEL_SIZE") == 0)) {
      if (arg_val == (char *)NULL) {
          error_string = "null output pixel size";
          break;
      } else {
        sprintf(s, "%s=%s", arg_id, arg_val);
        GetArgValArray(s, tmp_arr, &n);
        if (n > MAX_SDS_DIMS) {
          sprintf(msg, "resamp: (warning) only the first %d pixel sizes "
                       "will be used (out of %d supplied by the user), "
                       "since %d is the maximum number of SDSs allowed "
                       "in an HDF file. \n", MAX_SDS_DIMS, n, MAX_SDS_DIMS);
	  LogInfomsg(msg);
          n = MAX_SDS_DIMS;
        }
      }
      loc_st = true;
      for (ip = 0; ip < n; ip++) {
        if (sscanf(tmp_arr[ip], "%lg",
            &this->output_pixel_size[ip]) != 1) {
          sprintf(msg, "resamp: invalid output space pixel size "
                       "value ([%d]=%s).\n", ip, tmp_arr[ip]);
	       LogInfomsg(msg);
          loc_st = false;
        } else if (this->output_pixel_size[ip] <= 0.0) {
          sprintf(msg, "resamp: output space pixel size out of valid "
                  "range ([%d]=%s).\n", ip, tmp_arr[ip]);
	       LogInfomsg(msg);
          loc_st = false;
        }
      }
      if (!loc_st) {
        error_string = "invalid pixel size parameters";
        break;
      }
    }

    else if ((strcmp(arg_id, "OUL") == 0)  ||
             ((len_arg_id >= 30)  &&
              (strncmp(arg_id, "OUTPUT_SPACE_UPPER_LEFT_CORNER", 30) == 0))) {
      if (arg_val == (char *)NULL) {
          error_string = "null output space upper left corner";
          break;
      } else {
        sprintf(s, "%s=%s", arg_id, arg_val);
        GetArgValArray(s, tmp_arr, &n);
        if (n != 2) {
          sprintf(msg, "resamp: invalid output space upper left "
                  "corner (%s).\n", arg_val);
	  LogInfomsg(msg);
          error_string = "invalid UL corner";
          break;
        } else {
          if ((sscanf(tmp_arr[0], "%lg",
                      &this->output_space_def.ul_corner.x) != 1)  ||
              (sscanf(tmp_arr[1], "%lg",
                      &this->output_space_def.ul_corner.y) != 1)) {
            sprintf(msg, "resamp: invalid output space upper left "
                    "corner (%s).\n", arg_val);
	    LogInfomsg(msg);
            error_string = "invalid UL corner";
            break;
          } else
            this->output_space_def.ul_corner_set = true;
        }
      }
    }

    else if ((strcmp(arg_id, "OLR") == 0)  ||
             ((len_arg_id >= 31)  &&
              (strncmp(arg_id, "OUTPUT_SPACE_LOWER_RIGHT_CORNER", 31) == 0))) {
      if (arg_val == (char *)NULL) {
          error_string = "null output space upper left corner";
          break;
      } else {
        sprintf(s, "%s=%s", arg_id, arg_val);
        GetArgValArray(s, tmp_arr, &n);
        if (n != 2) {
          sprintf(msg, "resamp: invalid output space lower right "
                  "corner (%s).\n", arg_val);
	  LogInfomsg(msg);
          error_string = "invalid LR corner";
          break;
        } else {
          if ((sscanf(tmp_arr[0], "%lg",
                      &this->output_space_def.lr_corner.x) != 1)  ||
              (sscanf(tmp_arr[1], "%lg",
                      &this->output_space_def.lr_corner.y) != 1)) {
            sprintf(msg, "resamp: invalid output space lower right "
                    "corner (%s).\n", arg_val);
	    LogInfomsg(msg);
            error_string = "invalid LR corner";
            break;
          } else
            this->output_space_def.lr_corner_set = true;
        }
      }
    }

    else if ((strcmp(arg_id, "OSST") == 0) ||
             ((len_arg_id >= 26)  &&
              (strncmp(arg_id, "OUTPUT_SPATIAL_SUBSET_TYPE", 26) == 0))) {
      if (arg_val == (char *)NULL) {
          error_string = "null output spatial subset type";
      } else {
        strupper(arg_val);
        if (strcmp(arg_val, "LAT_LONG") == 0) {
          this->output_spatial_subset_type = LAT_LONG;
        } else if (strcmp(arg_val, "PROJ_COORDS") == 0) {
          this->output_spatial_subset_type = PROJ_COORDS;
        } else if (strcmp(arg_val, "LINE_SAMPLE") == 0) {
          this->output_spatial_subset_type = LINE_SAMPLE;
        }
        else {
          sprintf(msg, "resamp: invalid spatial subset type value (%s).\n", 
	          arg_val);
	  LogInfomsg(msg);
          error_string = "invalid spatial subset type";
	  break;
        }
      }
    }

    else if ((strcmp(arg_id, "OSP") == 0)  ||
             (strcmp(arg_id, "OUTPUT_PROJECTION_SPHERE") == 0)) {
      if (arg_val == (char *)NULL) {
        error_string = "null output projection sphere";
        break;
      } else {
        if (sscanf(arg_val, "%d", &this->output_space_def.sphere) != 1) {
          sprintf(msg, "resamp: invalid output space sphere "
	          "number (%s).\n", arg_val);
	  LogInfomsg(msg);
          error_string = "invalid output projection sphere";
          break;
        } else if (this->output_space_def.sphere < 0) {
          sprintf(msg, "resamp: output space sphere number out "
  	          "of valid range (%s).\n", arg_val);
	  LogInfomsg(msg);
          error_string = "invalid output projection sphere";
          break;
        }
      }
    }

    else if ((strcmp(arg_id, "OZN") == 0)  ||
             (strcmp(arg_id, "OUTPUT_PROJECTION_ZONE") == 0)) {
      if (arg_val == (char *)NULL) {
        error_string = "null output projection zone";
        break;
      } else {
        if (sscanf(arg_val, "%d", &this->output_space_def.zone) != 1) {
          sprintf(msg, "resamp: invalid output space zone "
	          "number (%s).\n", arg_val);
	  LogInfomsg(msg);
          error_string = "invalid output projection zone";
          break;
        } else this->output_space_def.zone_set = true;
      }
    }

    else if ((strcmp(arg_id, "IUL") == 0)  ||
             ((len_arg_id >= 29)  &&
              (strncmp(arg_id, "INPUT_SPACE_UPPER_LEFT_CORNER", 29) == 0))) {
      if (arg_val == (char *)NULL) {
          error_string = "null input space upper left corner";
          break;
      } else {
        sprintf(s, "%s=%s", arg_id, arg_val);
        GetArgValArray(s, tmp_arr, &n);
        if (n != 2) {
          sprintf(msg, "resamp: invalid input space upper left "
	          "corner (%s).\n", arg_val);
	  LogInfomsg(msg);
          error_string = "invalid input space UL corner";
          break;
        } else {
          if ((sscanf(tmp_arr[0], "%lg", 
	              &this->input_space_def.ul_corner.x) != 1)  ||
	      (sscanf(tmp_arr[1], "%lg", 
	              &this->input_space_def.ul_corner.y) != 1)) {
            sprintf(msg, "resamp: invalid input space upper left "
	            "corner (%s).\n", arg_val);
	    LogInfomsg(msg);
            error_string = "invalid input space UL corner";
            break;
	  } else 
	    this->input_space_def.ul_corner_set = true;
        }
      }
    }

    else if ((strcmp(arg_id, "OTY") == 0)  ||
             ((len_arg_id >= 16)  &&
              (strncmp(arg_id, "OUTPUT_DATA_TYPE", 16) == 0))) {
      if (arg_val == (char *)NULL) {
        error_string = "null output data type";
        break;
      } else {
        strupper(arg_val);
        if (strcmp(arg_val, "CHAR8") == 0) 
          this->output_data_type = DFNT_CHAR8;
        else if (strcmp(arg_val, "UINT8") == 0) 
          this->output_data_type = DFNT_UINT8;
        else if (strcmp(arg_val, "INT8") == 0) 
          this->output_data_type = DFNT_INT8;
        else if (strcmp(arg_val, "INT16") == 0) 
          this->output_data_type = DFNT_INT16;
        else if (strcmp(arg_val, "UINT16") == 0) 
          this->output_data_type = DFNT_UINT16;
        else if (strcmp(arg_val, "INT32") == 0)
          this->output_data_type = DFNT_INT32;
        else if (strcmp(arg_val, "UINT32") == 0) 
          this->output_data_type = DFNT_UINT32;
        else {
          sprintf(msg, "resamp: output data type (%s).\n", arg_val);
	  LogInfomsg(msg);
          error_string = "invalid output data type";
          break;
        }
      }
    }

    else {
      sprintf(msg, "resamp: invalid option (%s)\n", arg_id);
      LogInfomsg(msg);
      error_string = "invalid parameter";
      break;
    }

    if (arg_val != NULL)
      free(arg_val);
  }  /* end while (... */

  /* did the previous line have an error?  if so, then it automatically
     jumps out of the loop */
  if (error_string != (char *)NULL) {
    sprintf(msg, "resamp: %s.", error_string);
    LogInfomsg(msg);
    st = false;

    if (arg_val != NULL)
      free(arg_val);
  }

  Free2D((void **)tmp_arr);

  return st;
}

/* 
!C******************************************************************************

!Description: 'removeDoubleQuotes' is a simple function that will remove
              double quotes from a string array in place.
 
!Input Parameters:
 str		input line from parameter file

!Output Parameters:
 (returns)      the pointer of the srting passed in.

!Team Unique Header:

! Design Notes:

!END****************************************************************************
*/
char *removeDoubleQuotes( char *str ) {
   char *ptr, *nptr;
   if( str == 0 )
      return 0;

   for( ptr = nptr = str; *ptr != 0; ++ptr ) {
      if( *ptr == '"' )
         continue;
      *nptr = *ptr;
      ++nptr;
   }
   *nptr = 0;
   return str;
}



/***********************************************************************
 * int CleanupLine(
 *    char *line     (B) String or line to clean up.  The string is
 *                       modified.
 *    )
 *
 * Return values:
 *
 *  
 * (returns)      0, if error occurred
 *                strlen of output line, if no error occurred
 *
 *
 */
int CleanupLine(char* line)
{
    size_t  i;
    size_t  j;
    int     infile  = false;      /* is this the INPUT_FILENAME line?  */
    int     outfile = false;      /* is this the OUTPUT_FILENAME line? */
    int     found;
    bool    done;
    char    tmpstr[LINE_BUFSIZ];
    char*   capbuf = NULL;
    int     err = 0;

    /*
    --------------------------------------------------
    Check for blank line
    --------------------------------------------------*/
    if ( line == NULL || strlen( line ) == 0 )
        return 0;

    /*
    --------------------------------------------------
    Is this the INPUT_FILENAME line?
    --------------------------------------------------*/
    capbuf = strdup( line );
    strupper(capbuf);

    if ((strstr(capbuf, "INPUT_FILENAME")) != NULL)
    {
        infile = true;
    }

    if ((strstr(capbuf, "OUTPUT_FILENAME")) != NULL)
    {
        outfile = true;
    }

    free(capbuf);


    /*
    --------------------------------------------------
    Expand environment variables: Expects to find 
    "$(ENV_VAR)" where ENV_VAR is the environment 
    variable that one is looking for. 
    --------------------------------------------------*/
    strcpy (tmpstr, "");

    for (i = 0; (line[i] != '\0') && !err; ++i) 
    {
	if (line[i] == '$' && line[i+1] == '(') 
	{
	    j = i + 1;

	    if (line[j + 1] == '\0') 
	    {
		err = 1;
		break;
	    }

	    found = false;

	    for (j = j + 1; (line[j] != '\0') && (err == 0); ++j) 
	    {
		if ( line[j] == ')' ) 
		{
		     found = true;
		     break;
		}
	    }


	    if (found) 
	    {
		char*  envptr;
		size_t envnamelen = j - i + 1;

		strncpy(tmpstr, &line[i+2], envnamelen - 3);
		tmpstr[envnamelen - 3] = '\0';
		envptr = getenv(tmpstr);

		if (envptr == NULL) 
		{
		    strcpy( line, tmpstr );
		    err = 2;
		} 
		else 
		{
		    size_t envlen = strlen(envptr);

		    if (strlen(line) - envnamelen + envlen < LINE_BUFSIZ) 
		    {
			strncpy(tmpstr, line, i);
			tmpstr[i] = 0;
			strcat(tmpstr, envptr);
			strcat(tmpstr, &line[i + envnamelen]);
			strcpy(line, tmpstr);

			i = i + envlen;
		    } 
		    else
		       err = 1;
		}
	    } 
	    else
	       err = 1;
	}
    }


    /*
    --------------------------------------------------
    Cleanup the line
    --------------------------------------------------*/
    done = false;
    strcpy (tmpstr, "");

    j = 0;
    for ( i = 0, j = 0; i < strlen(line); i++ )
    {
	/*
	--------------------------------------------------
        Only process printable characters (disregard ^M)
	--------------------------------------------------*/
        if ( !isprint( line[i] ) )
        {
            tmpstr[j++] = '\0';
            break;
        }

        switch ( line[i] )
        {
	    /*
	    --------------------------------------------------
            Get rid of comments and newlines, and terminate 
	    line
	    --------------------------------------------------*/
            case '#':
            case '\n':
            case '\r':
            case '\0':
                tmpstr[j++] = '\0';
                done = true;
                break;

	    /*
	    --------------------------------------------------
            Otherwise just copy input char
	    --------------------------------------------------*/
            default:
                tmpstr[j++] = line[i];
                break;
        }

	/*
	--------------------------------------------------
        Are we done?
	--------------------------------------------------*/
        if (done)
            break;
    }


    /*
    --------------------------------------------------
    Store cleaned up line and return its length
    --------------------------------------------------*/
    if (!err)
    {
	strcpy(line, tmpstr);
        return strlen(line);
    }
    else
        return(0);


} /* CleanupLine */




int GetProjNum(char *proj_str)
/* 
!C******************************************************************************

!Description: 'GetProjNum' converts a projection id string to projection 
 number.
 
!Input Parameters:
 proj_str       projection id string

!Output Parameters:
 (returns)      projection number or '-1' if the projection id string does 
                not contain a valid value

!Team Unique Header:

 ! Design Notes:
   1. Either a valid projection number or projection short name are acceptable.

!END****************************************************************************
*/
{
  int i, proj_num;

  proj_num = -1;
  for (i = 0; i < PROJ_NPROJ; i++) {
    if (strcasecmp(proj_str, Proj_type[i].short_name) == 0) {
      proj_num = Proj_type[i].num;
      break;
    }
  } 

  if (proj_num == -1) {
    if (sscanf(proj_str, "%d", &proj_num) != 1) proj_num = -1;
  }

  return proj_num;
}


bool IsArgID(const char *arg_str, char *arg_id)
/* 
!C******************************************************************************

!Description: 'IsArgID' checks a complete input option string for a specific
 option id. 
 
!Input Parameters:
 arg_str        complete input option string
 arg_id         option id string

!Output Parameters:
 (returns)      status:
                  'true' if the 'arg_id' is contained in 'arg_str'
		  'false' otherwise

!Team Unique Header:

 ! Design Notes:
   1. The option id string must contain a equals ('=') character.
   2. The option string must be before the equals ('=') character.
   3. The routine is case sensitive.

!END****************************************************************************
*/
{
  int p1;

  p1 = charpos(arg_str, '=', 0);
  if (p1 == -1) return false;
  if (strncmp(arg_str, arg_id, p1) == 0)
    return true;
  else return false;
}


void GetArgValArray(const char *arg_str, char **arg_val, int *arg_cnt)
/* 
!C******************************************************************************

!Description: 'GetArgValArray' parses a complete input option string for a 
 set of option values. 
 
!Input Parameters:
 arg_str        complete input option string

!Output Parameters:
 arg_val        array of option values
 arg_cnt        number of values in the 'arg_val'

!Team Unique Header:

 ! Design Notes:
   1. The option string must contain a equals ('=') character.
   2. The option values must be after the equals ('=') character.
   3. A maximum of 'MAX_NUM_PARAM' values are returned.
   4. A warning is written to 'stderr' if more than 'MAX_NUM_PARAM' values are 
      given.
   5. The option values can either be separated by a comma (',') or by 
      spaces (' ').
   6. Separator characters are removed.
   7. Each option value (including extra spaces) can be at most 
      'MAX_OPTION_VAL_LEN - 1' characters.
   8. A warning is written to 'stderr' if an option value containing more 
      than 'MAX_OPTION_VAL_LEN - 1' characters is given. 

!END****************************************************************************
*/
{
  int p1, p2,  pb, len, arg_val_len;
  char msg[M_MSG_LEN+1];

  *arg_cnt = 0;
  len = (int)strlen(arg_str);
  p1 =  0;
  p2 = charpos(arg_str, '=', p1);
  if (p2 == -1) return;

  p1 = p2 + 1;
  p2 = charpos(arg_str, ',', p1);
  pb = charpos(arg_str, ' ', p1);
  if ((pb != -1)  && 
      (p2 == -1  ||  pb < p2)) p2 = pb;

  while ((p2 != -1) && (*arg_cnt < MAX_NUM_PARAM)) {
    arg_val_len = p2 - p1;
    if (arg_val_len > (MAX_OPTION_VAL_LEN - 1)) {
      sprintf(msg, "resamp: (warning) option value contains too many "
              "characters (%s), considering only "
	      "first %d characters\n", arg_str, MAX_OPTION_VAL_LEN);
      LogInfomsg(msg);
    }
    strmid(arg_str, p1, arg_val_len, arg_val[*arg_cnt]);
    strtrim(arg_val[*arg_cnt]);
    ++*arg_cnt;

    p1 = p2 + 1;
    while (p1 < len) {
      if (arg_str[p1] != ' ') break;
      p1++;
    }
    p2 = charpos(arg_str, ',', p1);
    pb = charpos(arg_str, ' ', p1);
    if ((pb != -1)  && 
        (p2 == -1  ||  pb < p2)) p2 = pb;
  }

  if (*arg_cnt >= MAX_NUM_PARAM) {
    sprintf(msg, "resamp: (warning) too many parameters "
            "in option (%s), considering only "
            "first %d parameter values\n", arg_str, MAX_NUM_PARAM);
    LogInfomsg(msg);
  } else {
    if (p1 < len) {
      arg_val_len = len - p1;
      if (arg_val_len > (MAX_OPTION_VAL_LEN - 1)) {
        sprintf(msg, "resamp: (warning) option value contains too many "
                "characters (%s), considering only "
	        "first %d characters\n", arg_str, MAX_OPTION_VAL_LEN);
	LogInfomsg(msg);
      }
      strmid(arg_str, p1, arg_val_len, arg_val[*arg_cnt]);
      strtrim(arg_val[*arg_cnt]);
      ++*arg_cnt;
    }
  }

  return;
}


char *GetArgVal(const char *arg_str)
/* 
!C******************************************************************************

!Description: 'GetArgVal' parses a complete input option string for a single 
 option value. 
 
!Input Parameters:
 arg_str        complete option string

!Output Parameters:
 (returns)      option value string or NULL when error occurs

!Team Unique Header:

 ! Design Notes:
   1. The option string must contain a equals ('=') character.
   2. The option value must be after the equals ('=') character.
   3. An error status is returned when there is an error allocating memory.
   4. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   5. Memory is allocated for the option value string.

!END****************************************************************************
*/
{
  int p1, i;
  int l1, len;
  char *tmp;

  p1 = charpos(arg_str, '=', 0);
  if (p1 == -1) return (char *)NULL;
  p1 = p1 + 1;
  len = (int)strlen(arg_str);
  l1 = len - p1 + 1;

  tmp = (char *)calloc((l1 + 1), sizeof(char));
  if (tmp == (char *)NULL) 
    LOG_RETURN_ERROR("unable to allocate memory", "GetArgVal", (char *)NULL);

  for (i = 0; i < l1; i++, p1++)
    tmp[i] = arg_str[p1];
  tmp[i] = '\0';
  return tmp;
}


void **Calloc2D(size_t nobj1, size_t nobj2, size_t size)
/* 
!C******************************************************************************

!Description: 'Calloc2D' allocates memory for a two dimensional array. 
 
!Input Parameters:
 p1             two dimensional array
 nobj1          size of the first dimension
 nobj2          size of the second dimension
 size           size of an array element (bytes)

!Output Parameters:
 (returns)      memory for the two dimensional array or NULL when error occurs

!Team Unique Header:

 ! Design Notes:
   1. The two dimensional array memory should be freed with the 'Free2D' 
      routine.
   2. An error status is returned when there is an error allocating memory.
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END****************************************************************************
*/
{
  void **p1;
  void *p2;
  size_t iobj1;
  char *c2;

  p2 = calloc((nobj1 * nobj2), size);
  if (p2 == NULL)
    LOG_RETURN_ERROR("unable to allocate memory (a)", "Calloc2D",(void **)NULL);

  p1 = (void **) calloc(nobj1, sizeof(void *));
  if (p1 == NULL) {
    free(p2);
    LOG_RETURN_ERROR("unable to allocate memory (b)", "Calloc2D",(void **)NULL);
  }

  c2 = (char *) p2;
  for (iobj1 = 0; iobj1 < nobj1; iobj1++) {
    p1[iobj1] = (void *) c2;
    c2 += (nobj2 * size);
  }

  return p1;
}                                        


void Free2D(void **p1)
/* 
!C******************************************************************************

!Description: 'Free2D' frees memory for a two dimensional array. 
 
!Input Parameters:
 p1             two dimensional array

!Output Parameters:
 (none)

!Team Unique Header:

 ! Design Notes:
   1. The two dimensional array memory allocation should be made with 
      the 'Calloc2D' routine.

!END****************************************************************************
*/
{
  if (*p1 != NULL) free(*p1);
  if (p1 != NULL) free(p1);
  return;
}                  


int charpos(const char *s, char c, int p)
/* 
!C******************************************************************************

!Description: 'charpos' returns the location of the first occurrence of a
 character in a string. 
 
!Input Parameters:
 s              string to be searched
 c              character to be searched for
 p              starting position in the string to be searched

!Output Parameters:
 (returns)      character position in the string or '-1' when the character is 
                not found

!Team Unique Header:

 ! Design Notes:
   1. This routine is case sensitive.

!END****************************************************************************
*/
{
  int len, i;

  len = (int)strlen(s);
  for (i = p; i < len; i++)
    if (s[i] == c) return i;
  return -1;
}


int strpos(const char *s1, const char *s2, int p)
/* 
!C******************************************************************************

!Description: 'strpos' returns the location of the first occurrence of a
 substring in a string. 
 
!Input Parameters:
 s1             string to be searched
 s2             substring to be searched for
 p              starting position in the string to be searched

!Output Parameters:
 (returns)      location of the start of the substring in the string 
                or '-1' when the substring is not found

!Team Unique Header:

 ! Design Notes:
   1. This routine is case sensitive.

!END****************************************************************************
*/
{
  int i, j, k, fn;
  int len1, len2;
  char tmp[MAX_SDS_STR_LEN];

  fn = 0;
  len1 = (int)strlen(s1);
  len2 = (int)strlen(s2);
  if (len1 >= len2)
  {
    for (i = p; i <= (len1 - len2); i++)
    {
      if (s1[i] == s2[0])
      {
        for (j = i, k = 0; k < len2; k++, j++)
          tmp[k] = s1[j];
        tmp[k] = '\0';
        if (strcmp(tmp, s2) == 0)
	{
          fn = 1;
          break;
        }
      }
    }
  }

  if (fn == 1) return i;
  else return -1;
}             


void strtrim(char *s)
/* 
!C******************************************************************************

!Description: 'strtrim' removes the leading and trailing blanks from a string. 
 
!Input Parameters:
 s              input string

!Output Parameters:
 s              string with blanks removed

!Team Unique Header:

 ! Design Notes:
   (none)

!END****************************************************************************
*/
{
  int i, j1, j2, k, len;

  len = (int)strlen(s);
  for (j1 = 0; j1 < len; j1++)
    if ((s[j1] != ' ') && (s[j1] != '\t') && (s[j1] != '\n')) break;
  for (j2 = (len - 1); j2 >= 0; j2--)
    if ((s[j2] != ' ') && (s[j2] != '\t') && (s[j1] != '\n')) break;
  for (i = j1, k = 0; i <= j2; i++, k++)
    s[k] = s[i];
  s[k] = '\0';
}                  


void strupper(char *s)
/* 
!C******************************************************************************

!Description: 'strupper' converts a string to upper case. 
 
!Input Parameters:
 s              input string

!Output Parameters:
 s              string converted to upper case

!Team Unique Header:

 ! Design Notes:
   (none)

!END****************************************************************************
*/
{
  int i, len;

  len = (int)strlen(s);
  for (i = 0; i < len; i++)
    s[i] = (char)toupper((int)s[i]);

  return;
}                  

void strmid(const char *s1, int p1, int cnt, char *s2)
/* 
!C******************************************************************************

!Description: 'strmid' extracts a substring from a string. 
 
!Input Parameters:
 s1             input string
 p1             starting position of the substring in the string
 cnt            length of the substring

!Output Parameters:
 s2             extracted substring

!Team Unique Header:

 ! Design Notes:
   1. The length of the substring must be less then the memory allocated for the
      substring.

!END****************************************************************************
*/
{
  int i, j, p2;

  p2 = p1 + cnt;
  for (i = p1, j = 0; i < p2; i++, j++)
    s2[j] = s1[i];
  s2[j] = '\0';
}

       
bool parse_sds_name(Param_t *this, char *sds_string)
/* 
!C******************************************************************************

!Description: 'parse_sds_name' parses the SDS name string.
 
!Input Parameters:
 this           'param' data structure; the following fields are input:
                  input_sds_name_list
 sds_string     character string containing the input SDS names

!Output Parameters:
 this           'param' data structure; the following fields are modified:
                  input_sds_name_list, num_input_sds
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

! Design Notes:
 1. Valid options are
    a. Nothing
    b. SDSname
    c. SDSname1; SDSname2; ...; SDSnameN
    d. SDSname, band1, band2, ... bandN
    e. SDSname1, band1, band2, ... bandN; SDSname2, band1, band2, ... bandN;
       ...; SDSnameN, band1, band2, ... bandN
    f. SDSname1; SDSname2, band1, ... bandN; SDSname3, ...
       or any other combination of the above
 2. band1, ... bandN are actually 0s or 1s representing whether that band
    should be processed (1) or not (0).  bands are 0 by default.

!END****************************************************************************
*/
{
  char *sdslist;         /* copy of the SDS string */
  char *sdsname;         /* pointer to the beginning of the SDS name */
  char *bandval;         /* pointer to the beginning of the band value */
  char *sdsend;          /* pointer to the end of the SDS name (;) */
  char *bandend;         /* pointer to the end of the band number (,) */
  bool process_bands;    /* are there bands to be processed for this SDS? */
  bool process_sds;      /* are there more SDSs to be processed? */
  int nsds;              /* number of SDSs listed in the SDS string */
  int nbands;            /* number of bands specified for the current SDS */

  /* check for an empty SDS list */
  if (sds_string == NULL || sds_string == "")
  {
    this->num_input_sds = 0;
    return true;
  }

  /* duplicate the SDS string for use in this routine */
  sdslist = strdup (sds_string);
  if (sdslist == NULL)
    LOG_RETURN_ERROR ("error duplicating SDS list", "parse_sds_name", false);

  /* loop through the SDS list */
  nsds = 0;
  process_sds = true;
  while (process_sds)
  {
    /* get the next SDS name */
    sdsname = sdslist;

    /* search for the band break ',' */
    bandend = strchr (sdslist, ',');

    /* search for the SDS name break ';' */
    sdsend = strchr (sdslist, ';');

    /* are there bands in this to process? */
    if (bandend != NULL && sdsend != NULL && bandend < sdsend)
      process_bands = true;
    else if (bandend != NULL && sdsend == NULL)
      process_bands = true;
    else
      process_bands = false;

    /* are there more SDSs to process? */
    if (sdsend != NULL)
      process_sds = true;
    else
      process_sds = false;

    /* process the bands */
    if (process_bands)
    { /* this SDS contains a band list */
      /* get the SDS name which ends at the comma */
      sdslist = bandend + 1;
      *bandend = '\0';

      /* strip the blank spaces from the beginning and end of the
         SDS name */
      if (!strip_blanks (sdsname))
        LOG_RETURN_ERROR ("error removing blanks from the SDS name",
          "parse_sds_name", false);
      strcpy (this->input_sds_name_list[nsds], sdsname);
 
      /* process the band list (0s and 1s) */
      /* sdslist points to the first band value */
      nbands = 0;
      while (process_bands)
      {
        /* get the next band value (0 or 1) */
        bandval = sdslist;

        /* search for the band break ',' */
        bandend = strchr (sdslist, ',');

        /* search for the SDS name break ';' */
        sdsend = strchr (sdslist, ';');

        /* are there more bands in this SDS to process? */
        if (bandend != NULL && sdsend != NULL && bandend < sdsend)
          process_bands = true;
        else if (bandend != NULL && sdsend == NULL)
          process_bands = true;
        else
          process_bands = false;

        if (process_bands)
        { /* more bands to process */
          /* get the band value, which ends at the comma */
          sdslist = bandend + 1;
          *bandend = '\0';
          this->input_sds_bands[nsds][nbands++] = atoi (bandval);
        }
        else if (process_sds)
        { /* more SDSs to process */
          /* get the band value, which ends at the semi-colon */
          sdslist = sdsend + 1;
          *sdsend = '\0';
          this->input_sds_bands[nsds][nbands++] = atoi (bandval);
        }
        else
        { /* no more SDSs to process */
          /* get the band value, which ends at the end of the string */
          this->input_sds_bands[nsds][nbands++] = atoi (bandval);
        }
      }
       
      /* store the number of bands that the user specified */
      this->input_sds_nbands[nsds] = nbands;

      /* increment the count for the SDSs */
      nsds++;
    }
    else if (process_sds)
    { /* more SDSs to process but no bands to process */
      /* get the SDS name which ends at the semi-colon */
      sdslist = sdsend + 1;
      *sdsend = '\0';

      /* strip the blank spaces from the beginning and end of the
         SDS name */
      if (!strip_blanks (sdsname))
        LOG_RETURN_ERROR ("error removing blanks from the SDS name",
          "parse_sds_name", false);
      strcpy (this->input_sds_name_list[nsds++], sdsname);
    }
    else
    { /* no more SDSs to process and no bands to process */
      /* get the SDS name which ends at the end of the string */
      /* strip the blank spaces from the beginning and end of the
         SDS name */
      if (!strip_blanks (sdsname))
        LOG_RETURN_ERROR ("error removing blanks from the SDS name",
          "parse_sds_name", false);
      strcpy (this->input_sds_name_list[nsds++], sdsname);
    }
  } /* while (process_sds) */

  this->num_input_sds = nsds;
  return true;
}


bool strip_blanks(char *instr)
/*
!C******************************************************************************
!Description: 'strip_blanks' removes the blank spaces from the beginning and
 end of the input character string

!Input Parameters:
 char *         input string

!Output Parameters:
 char *         input string with blank spaces removed from beginning and
                  end of the string
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

!Design Notes:

!END*****************************************************************************/
{
  int i;
  char *tmpstr;      /* temporary storage for the string */

  /* allocate space for the temporary string holder */
  tmpstr = (char *) malloc ((strlen (instr) + 1) * sizeof (char));
  if (tmpstr == NULL)
    LOG_RETURN_ERROR ("error allocating space for the input string",
      "strip_blanks", false);

  /* loop through the string and find the first non-blank character */
  for (i = 0; i < (int) strlen (instr); i++)
  {
    if (instr[i] != ' ')
      break;
  }

  /* copy the input string starting at the first non-blank character */
  strcpy (tmpstr, &instr[i]);

  /* loop through the string from the end and find the first non-blank
     character */
  for (i = strlen (tmpstr) - 1; i >= 0; i--)
  {
    if (tmpstr[i] != ' ')
      break;
  }

  /* put an end of string character after the last non-blank character */
  tmpstr[i+1] = '\0';

  /* copy the new string to instr */
  strcpy (instr, tmpstr);

  return true;
}


bool update_sds_info(int sdsnum, Param_t *this)
/* 
!C******************************************************************************

!Description: 'update_sds_info' initializes the SDS names, rank and dimension
 index information for the specified SDS number.
 
!Input Parameters:
 sdsnum         number if the SDS to be updated
 this           'param' data structure; the following fields are input:
                  input_sds_name

!Output Parameters:
 this           'param' data structure; the following fields are modified:
                  input_sds_name, output_sds_name, rank, dim
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. If the SDS name is followed by a comma and one numeric value a rank of 
      three dimensions is assumed.  If the SDS name is followed by a commas 
      and two numeric values a rank of four dimensions is assumed.  Otherwise,
      a rank of two dimensions is assumed.
   2. The size of the first two dimensions is not modified.

!END****************************************************************************
*/
{
  char n_str[5];
  int p1, p2, len, ir;
  char temp_string[20];

  this->rank[sdsnum] = 2;
  if ((p1 = charpos(this->input_sds_name, ',', 0)) != -1)
  {
    len = (int)strlen(this->input_sds_name);
    if ((p2 = charpos(this->input_sds_name, ',', p1+1)) == -1) {
      this->rank[sdsnum] = 3;
      strmid(this->input_sds_name, p1+1, len-p1-1, n_str);
      this->dim[sdsnum][2] = (int)atoi(n_str);
    }
    else {
      this->rank[sdsnum] = 4;
      strmid(this->input_sds_name, p1+1, p2-p1-1, n_str);
      this->dim[sdsnum][2] = (int)atoi(n_str);
      strmid(this->input_sds_name, p2+1, len-p2-1, n_str);
      this->dim[sdsnum][3] = (int)atoi(n_str);
    }
    this->input_sds_name[p1] = '\0';
  }

  len = (int)strlen(this->input_sds_name) + ((this->rank[sdsnum] - 2) * 10);
  this->output_sds_name = (char *)calloc((len + 1), sizeof(char));
  if (this->output_sds_name == (char *)NULL)
    LOG_RETURN_ERROR("unable to allocate memory", "update_sds_info", false);

  strcpy(this->output_sds_name, this->input_sds_name);

  for (ir = 2; ir < this->rank[sdsnum]; ir++) {
    sprintf(temp_string, "_b%d", this->dim[sdsnum][ir]);
    if ((int)(strlen(this->output_sds_name) + strlen(temp_string)) > len)
      LOG_RETURN_ERROR("output sds name too long", "update_sds_info", false);
    strcat(this->output_sds_name, temp_string);
  }

  return true;
}


int get_line(FILE *fp, char *s)
/* 
!C******************************************************************************

!Description: 'get_line' reads a line from a file. 
 
!Input Parameters:
 fp             file handle

!Output Parameters:
 s              the line read from the file
 (returns)      length of the line or zero for end-of-file

!Team Unique Header:

 ! Design Notes:
   1. A maximum of 'MAX_SDS_STR_LEN' characters is returned. We need to handle
      the SDS list, which can be very large.

!END****************************************************************************
*/
{
  int i, c;

  i = 0;
  while ((c = fgetc(fp)) != EOF)
  {
    s[i++] = c;
    if (c == '\n') break;
    if (i >= MAX_SDS_STR_LEN) break;
  }
  s[i-1] = '\0';
  return i;
}

bool NeedHelp(int argc, const char **argv)
/* 
!C******************************************************************************

!Description: 'NeedHelp' determines if the user needs help and if so, prints 
 out the help or usage information.
 
!Input Parameters:
 argc           number of command line arguments
 argv           command line argument list

!Output Parameters:
 (returns)      status:
                  'true' = user needed help
		  'false' = the user doesn't need help

!Team Unique Header:

 ! Design Notes:
   1. This routine exits when an error exit occurs.
   2. An error occurs when:
       a. the user does not enter any parameters
       b. an error occurs getting the argument value
       c. an invalid projection id is given
   3. The help and usage information is written to 'stderr'.

!END****************************************************************************
*/
{
  char *tmp;
  char msg[M_MSG_LEN+1];
  int itmp;
  int i, iproj, itype;

  if (argc > 2) return (false);

  if (argc < 2) {
    LogInfomsg(USAGE);
    LOG_ERROR("getting runtime parameters", "NeedHelp");
  }

  if ((strcmp(argv[1], "-h") == 0)  || 
      (strcmp(argv[1], "-help") == 0)) {
    LogInfomsg(HELP);
    return (true);
  }

  if (IsArgID(argv[1], "-h")  ||  IsArgID(argv[1], "-help")) {
    tmp = GetArgVal(argv[1]);
    if (tmp == (char *)NULL) {
      LogInfomsg(USAGE);
      LOG_ERROR("can't get argument value (-help)", "NeedHelp");
    }
    strupper(tmp);
  } else {
    return (false);
  }

  if (strcmp(tmp, "PROJ") == 0) {
    LogInfomsg(GENERAL_PROJ_HEADER);

    LogInfomsg("\n Projections (number, short name, name):\n");
    for (i = 0; i < PROJ_NPROJ; i++) {
      sprintf(msg, "  %2d  %-6s  %s\n", Proj_type[i].num, 
             Proj_type[i].short_name, Proj_type[i].name);
      LogInfomsg(msg);
    }

    LogInfomsg(
           "\n Spheres (number, semi-major axis, semi-minor axis, name):\n");
    for (i = 0; i < PROJ_NSPHERE; i++) {
      sprintf(msg, "  %2d %15.6f %15.6f  %s\n", i, Proj_sphere[i].major_axis, 
             Proj_sphere[i].minor_axis, Proj_sphere[i].name);
      LogInfomsg(msg);
    }

    LogInfomsg(GENERAL_PROJ_TRAILER);
    free(tmp);
    return (true);
  }

  if (strcmp(tmp, "PARAM") == 0) {
    LogInfomsg(GENERAL_PARAM);
    free(tmp);
    return (true);
  }

  itmp = GetProjNum(tmp);
  free(tmp);
  if (itmp < 0) {
    sprintf(msg, "resamp: invalid projection number (%s).\n", argv[1]);
    LogInfomsg(msg);
    LOG_ERROR("invalid argument value", "NeedHelp");
  }

  iproj = -1;
  for (i = 0; i < PROJ_NPROJ; i++) {
    if (itmp == Proj_type[i].num) {
      iproj = i;
      break;
    }
  }
  if (iproj < 0) 
    LOG_ERROR("invalid projection number", "NeedHelp");

  sprintf(msg, "\n %s Projection\n", Proj_type[iproj].name);
  LogInfomsg(msg);
  sprintf(msg, "   Number %d\n   Short name %s\n", Proj_type[iproj].num, 
         Proj_type[iproj].short_name);
  LogInfomsg(msg);

  sprintf(msg, "\n Projection parameters (number, parameter name):\n");
  LogInfomsg(msg);
  for (i = 0; i < PROJ_NPARAM; i++) {
    itype = Proj_param_value_type[iproj][i];
    sprintf(msg, "  %2d  %s\n", i, Proj_param_type[itype].name);
    LogInfomsg(msg);
  }
  LogInfomsg("\n");

  return (true);
}
