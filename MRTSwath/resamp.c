/*
!C****************************************************************************

!File: resamp.c
  
!Description: This progam resamples a swath (L1 or L2) data set to an output
 grid.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.1 2002/03/02
 Robert Wolfe
 Added special handling for input ISIN case.

 Revision 1.2 2002/05/10
 Robert Wolfe
 Added separate output SDS name.

 Revision 1.5 2002/08/09
 Gail Schmidt
 Allow the user to specify the output UL/LR in lat/long, instead of the
 output UL and #lines/#samples in output projection space.

 Revision 1.5 2002/08/09
 Gail Schmidt
 Allow the user to specify output projection parameters in decimal degrees
 instead of DMS.

 Revision 1.5 2002/08/09
 Gail Schmidt
 Allow the user to output to GeoTiff (for GEO, ISIN, LAMAZ, TM, UTM, HAMMER,
 GOODE, LAMCC, MOLL, PS, and SNSOID). Each separate SDS/band is output to a
 separate GeoTiff file.

 Revision 1.5 2002/08/09
 Gail Schmidt
 Output UL, LR, #Lines, #Samples, Pixel Size, Proj Type, Proj Zone,
 Proj Parms, and GCTP Sphere # as global attributes to the output HDF file.

 Revision 1.5 2002/11/30
 Gail Schmidt
 Support Geographic output projection. Output pixel size must be specified
 in degrees for output to Geographic, instead of meters.

 Revision 2.0 2003/10/09
 Gail Schmidt
 Modified the software to process all the SDSs, if an SDS was not specified
 in the parameter file.
 Support output to raw binary.

 Revision 2.0 2003/12/13
 Gail Schmidt
 Added standard MRT printing messages.

 Revision 2.0 2004/01/07
 Gail Schmidt
 To support multiple resolutions for output to HDF, the different resolutions
 need to be written to separate HDF files.

 Revision 2.0a 2004/07/21
 Gail Schmidt
 Removed the '/'s from the SDSname before creating the output Raw Binary name.

 Revision 2.2 2008/10/28
 Maverick Merritt
 Increased precision from float to double mostly in the Img_coord_double_t
 structures (used to be called Img_coord_float_t), to solve a problem of output
 pixels being skipped during processing and thus were being set to "fill".
 Most noticeable during bilinear sampling.


!Team Unique Header:
  This software was developed by the MODIS Land Science Team Support 
  Group for the Labatory for Terrestrial Physics (Code 922) at the 
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
      phone: 301-614-5508               Lanham, MD 20706  

      Gail Schmidt
      USGS EROS                         SAIC
      MODIS Reprojection Tool           501 East St. Joesph Street
      gschmidt@usgs.gov                 Rapid City, SD 57701
  
 ! Design Notes:
   1. See 'USAGE' in 'parser.h' for information on how to use the program.
   2. A temporary output file ('patches.tmp') is created and used to stage data 
      during the resampling and can be deleted once the program exits.

!END****************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <sys/wait.h>
#endif
#include <ctype.h>
#if defined(__CYGWIN__) || defined(WIN32)
#include <getopt.h>
#else
#ifndef WIN32
#include <unistd.h>
#endif
#endif
#include <time.h>
#ifdef WIN32
#include "winpid.h"
#endif

#include "resamp.h"
#include "scan.h"
#include "param.h"
#include "geoloc.h"
#include "input.h"
#include "output.h"
#include "kernel.h"
#include "space.h"
#include "patches.h"
#include "bool.h"
#include "myhdf.h"
#include "myproj.h"
#include "const.h"
#include "parser.h"
#include "myerror.h"
#include "myisoc.h"
#include "geowrpr.h"
#include "rb.h"
#include "addmeta.h"
#include "logh.h"

/* Macros */

#define MAX2(a, b) (((a) >= (b)) ? (a) : (b))
#define MIN2(a, b) (((a) <= (b)) ? (a) : (b))

/* Type definitions */

typedef enum {FAILURE = 0, SUCCESS = 1} Status_t;

/* Functions */

int main (int argc, const char **argv) 
/* 
!C******************************************************************************

!Description: 'main' is the main function for the 'resamp' program.
 
!Input Parameters:
 argc           number of run-time (command line) arguments
 argv           list of run-time argument values

!Output Parameters:
 (returns)      status:
                  'EXIT_SUCCESS' = okay
		  'EXIT_FAILURE' = fatal error

!Team Unique Header:

 ! Design Notes:
   1. See 'USAGE' in 'parser.h' for information on how to use the program.
   2. An error status is returned when:
       a. there is a problem getting runtime parameters
       b. there is a bad input image, geolocation or kernel file
       c. unable to set up the input grid, scan data structure, 
          or the output space
       d. unable to generate kernel
       f. unable to free memory for data structures
       g. unable to set up intermediate patches data structure
       h. there is an error reading geolocation for a scan
       i. there is an error mapping a scan
       j. there is an error generating geolocation for a scan
       k. there is an error copying a scan
       l. there is an error extending the scan
       m. there is an error reading input data for a scan
       n. there is an error resampling a scan
       o. there is an error writting patches to disk
       p. there is an error closing input image, geolocation, kernel or 
          output files
       q. there is an error untouching patches
       r. there is an error creating output image file
       s. there is an error opening output file
       t. there is an error unscrambling the output file
       u. there is an error writing metadata.
   3. Errors are handled with the 'LOG_ERROR' macro.

!END****************************************************************************
*/
{
  int i,j,k;
  int curr_sds, curr_band;        /* current SDS and current band in SDS */
  char tmp_sds_name[MAX_STR_LEN];
  char errstr[M_MSG_LEN+1];              /* error string for OpenInput */
  char sdsname[256];              /* SDS name without '/'s */
  char msg[M_MSG_LEN+1];
  Param_t *param = NULL;
  Param_t *param_save = NULL;
  Geoloc_t *geoloc = NULL;
  Kernel_t *kernel = NULL;
  Input_t *input = NULL;
  Scan_t *scan = NULL;
  Space_t *output_space = NULL;
  Patches_t *patches = NULL;
  int iscan, kscan;
  int il, nl;
  Output_t *output = NULL;
  Img_coord_double_t img;
  Geo_coord_t geo;
  FILE_ID *MasterGeoMem;    /* Output GeoTiff file */
  FILE *rbfile = NULL;       /* Output Raw Binary file */
  char HDF_File[1024], CharThisPid[256], FinalFileName[1024];
  Output_t output_mem;       /* Contains output HDF file */
  int32 exec_resamp, ThisPid; 
  char filename[1024];       /* name of raw binary file to be written to */
  time_t startdate, enddate;  /* start and end date struct */
  bool file_created;         /* was the current HDF file created? */

  /* Initialize the log file */
  InitLogHandler();

  /* Print the MRTSwath header */
  LogInfomsg(
     "*******************************************************************"
     "***********\n");
  sprintf(msg, "%s (%s)\n", RESAMPLER_NAME, RESAMPLER_VERSION);
  LogInfomsg(msg);
  startdate = time(NULL);
  sprintf(msg, "Start Time:  %s", ctime(&startdate));
  LogInfomsg(msg);
  LogInfomsg(
  "------------------------------------------------------------------\n");

  /* Get runtime parameters */
  if (NeedHelp(argc, argv))
    exit(EXIT_SUCCESS);

  param_save = GetParam(argc, argv);
  if (param_save == (Param_t *)NULL)
    LOG_ERROR("getting runtime parameters", "main");

  /* Print out the user-specified processing information */
  PrintParam(param_save);

  /* Loop through all the SDSs */
  for (curr_sds = 0; curr_sds < param_save->num_input_sds; curr_sds++)
  {
    /* Loop through all the bands in the current SDS */
    for (curr_band = 0; curr_band < param_save->input_sds_nbands[curr_sds];
         curr_band++)
    {
      /* Is this band one that should be processed? */
      if (!param_save->input_sds_bands[curr_sds][curr_band])
        continue;

      /* Assume the HDF file does not need to be created */
      file_created = false;

      /* Get a copy of the user parameters */
      param = CopyParam(param_save);
      if (param == (Param_t *)NULL)
        LOG_ERROR("copying runtime parameters", "main");

      /* Create the input_sds_name which is "SDSname, band" */
      if (param->input_sds_nbands[curr_sds] == 1)
      {
        /* 2D product so the band number is not needed */
        sprintf(tmp_sds_name, "%s", param->input_sds_name_list[curr_sds]);
      }
      else
      {
        /* 3D product so the band number is required */
        sprintf(tmp_sds_name, "%s, %d", param->input_sds_name_list[curr_sds],
          curr_band);
      }

      param->input_sds_name = strdup (tmp_sds_name);
      if (param->input_sds_name == NULL)
        LOG_ERROR("error creating input SDS band name", "main");
      sprintf(msg, "\nProcessing %s ...\n", param->input_sds_name);
      LogInfomsg(msg);

      /* Update the system to process the current SDS and band */
      if (!update_sds_info(curr_sds, param))
        LOG_ERROR("error updating SDS information", "main");

      /* Open input file for the specified SDS and band */
      input = OpenInput(param->input_file_name, param->input_sds_name, 
        param->iband, param->rank[curr_sds], param->dim[curr_sds], errstr);
      if (input == (Input_t *)NULL) {
        /* This is an invalid SDS for our processing so skip to the next
           SDS. We will only process SDSs that are at the 1km, 500m, or
           250m resolution (i.e. a factor of 1, 2, or 4 compared to the
           1km geolocation lat/long data). We also only process CHAR8,
           INT8, UINT8, INT16, and UINT16 data types. */
        LOG_WARNING(errstr, "main");
        LOG_WARNING("not processing SDS/band", "main");
        break;
      }

      /* Setup kernel */
      kernel = GenKernel(param->kernel_type);
      if (kernel == (Kernel_t *)NULL)
        LOG_ERROR("generating kernel", "main");

      /* Open geoloc file */
      geoloc = OpenGeolocSwath(param->geoloc_file_name);
      if (geoloc == (Geoloc_t *)NULL)
        LOG_ERROR("bad geolocation file", "main");

      /* Setup input scan */
      scan = SetupScan(geoloc, input, kernel);
      if (scan == (Scan_t *)NULL)
        LOG_ERROR("setting up scan data structure", "main");

      /* Set up the output space, using the current pixel size and
         number of lines and samples based on the current SDS (pixel size
         and number of lines and samples are the same for all the bands
         in the SDS) */
      param->output_space_def.img_size.l = param->output_img_size[curr_sds].l;
      param->output_space_def.img_size.s = param->output_img_size[curr_sds].s;
      param->output_space_def.pixel_size = param->output_pixel_size[curr_sds];
      sprintf(msg, "  output lines/samples: %d %d\n",
        param->output_space_def.img_size.l, param->output_space_def.img_size.s);
      LogInfomsg(msg);
      if (param->output_space_def.proj_num == PROJ_GEO)
      {
        sprintf(msg, "  output pixel size: %.4f\n",
          param->output_space_def.pixel_size * DEG);
	LogInfomsg(msg);
      }
      else
      {
        sprintf(msg, "  output pixel size: %.4f\n",
          param->output_space_def.pixel_size);
	LogInfomsg(msg);
      }
      LogInfomsg("  output data type: ");
      switch (param->output_data_type)
      {
          case DFNT_CHAR8:
              LogInfomsg("CHAR8\n");
              break;
          case DFNT_UINT8:
              LogInfomsg("UINT8\n");
              break;
          case DFNT_INT8:
              LogInfomsg("INT8\n");
              break;
          case DFNT_UINT16:
              LogInfomsg("UINT16\n");
              break;
          case DFNT_INT16:
              LogInfomsg("INT16\n");
              break;
          case DFNT_UINT32:
              LogInfomsg("UINT32\n");
              break;
          case DFNT_INT32:
              LogInfomsg("INT32\n");
              break;
          default:
              LogInfomsg("same as input\n");
              break;
      }

      output_space = SetupSpace(&param->output_space_def);
      if (output_space == (Space_t *)NULL) 
        LOG_ERROR("setting up output space", "main");

      /* Compute and print out the corners */
      img.is_fill = false;
      img.l = img.s = 0.0;
      if (!FromSpace(output_space, &img, &geo)) 
      {
        LOG_WARNING("unable to compute upper left corner", "main");
      }
      else
      {
        sprintf(msg,
          "  output upper left corner: lat %13.8f  long %13.8f\n", 
          (DEG * geo.lat), (DEG * geo.lon));
	LogInfomsg(msg);
      }

      img.is_fill = false;
      img.l = output_space->def.img_size.l - 1; 
      img.s = output_space->def.img_size.s - 1;
      if (!FromSpace(output_space, &img, &geo)) 
      {
        LOG_WARNING("unable to compute lower right corner", "main");
      }
      else
      {
        sprintf(msg,
          "  output lower right corner: lat %13.8f  long %13.8f\n", 
          (DEG * geo.lat), (DEG * geo.lon));
        LogInfomsg(msg);
      }

      /* If output data type not specified, then set to input data type
         (changes from SDS to SDS) */
      if (param->output_data_type == -1) 
        param->output_data_type = input->sds.type;

      /* Save the data type of this SDS for output to the metadata */
      param_save->output_dt_arr[curr_sds] = param->output_data_type;

      /* Setup intermediate patches. Setup as the input data type. Then we
         will convert to the output data type later. */
      patches = SetupPatches(&param->output_space_def.img_size, 
        param->patches_file_name, input->sds.type, input->fill_value);
      if (patches == (Patches_t *)NULL) 
        LOG_ERROR("setting up intermediate patches data structure","main");

      if (param->input_space_type != SWATH_SPACE)
        LOG_ERROR("input space type is not SWATH", "main");

      sprintf(msg, "  input lines/samples: %d %d\n", input->size.l,
        input->size.s);
      LogInfomsg(msg);
      switch (input->ires)
      {
        case 1:
          LogInfomsg("  input resolution: 1 km\n");
          break;
        case 2:
          LogInfomsg("  input resolution: 500 m\n");
          break;
        case 4:
          LogInfomsg("  input resolution: 250 m\n");
          break;
      }
      LogInfomsg("  %% complete: 0%");

      /* For each input scan */
      kscan = 0;
      for (iscan = 0; iscan < geoloc->nscan; iscan++)
      {
        /* Update status? */
        if (100 * iscan / geoloc->nscan > kscan)
        {
          kscan = 100 * iscan / geoloc->nscan;
          if (kscan % 10 == 0)
          {
            sprintf(msg, " %d%%", kscan);
            LogInfomsg(msg);
	  }
        }

        /* Read the geolocation data for the scan and map to output space */
        if (!GetGeolocSwath(geoloc, output_space, iscan)) 
          LOG_ERROR("reading geolocation for a scan", "main");

        /* Map scan to input resolution */
        if (!MapScanSwath(scan, geoloc)) 
          LOG_ERROR("mapping a scan (swath)", "main");

        /* Extend the scan */
        if (!ExtendScan(scan)) LOG_ERROR("extending the scan", "main");

        /* Read input scan data into extended scan */
        il = iscan * input->scan_size.l;
        nl = input->scan_size.l;
        if (il + nl > input->size.l)
          nl = input->size.l - il;

        if (!GetScanInput(scan, input, il, nl))
          LOG_ERROR("reading input data for a scan", "main");

        /* Resample all of the points in the extended scan */
        if (!ProcessScan(scan, kernel, patches, nl, param->kernel_type))
          LOG_ERROR("resampling a scan", "main");

        /* Toss patches that were not touched */
        if (!TossPatches(patches, param->output_data_type))
          LOG_ERROR("writting patches to disk", "main");

      } /* End loop for each input scan */

      /* Finish the status message */
      LogInfomsg(" 100%\n");

      /* Save the background fill value from the patches data structure for
         output to the metadata */
      param_save->fill_value[curr_sds] = patches->fill_value;

      /* If output is raw binary, then we need the patches information
         so write the header before deleting the patches info */
      if (param->output_file_format == RB_FMT)
      { /* Output is raw binary */
        /* Create the raw binary header file */
        if (!WriteHeaderFile (param, patches))
          LOG_ERROR("writing raw binary header file", "main");
      }

      /* Done with scan and kernel strutures */
      if (!FreeScan(scan))
        LOG_ERROR("freeing scan structure", "main");
      if (!FreeKernel(kernel))
        LOG_ERROR("freeing kernel structure", "main");

      /* Close geolocation file */
      if (!CloseGeoloc(geoloc))
         LOG_ERROR("closing geolocation file", "main");

      /* Close input file */
      if (!CloseInput(input)) LOG_ERROR("closing input file", "main");

      /* Free the output space structure */
      if (!FreeSpace(output_space)) 
        LOG_ERROR("freeing output space structure", "main");

      /* Write remaining patches in memory to disk */
      if (!UntouchPatches(patches)) 
        LOG_ERROR("untouching patches", "main");
      if (!TossPatches(patches, param->output_data_type))
        LOG_ERROR("writting remaining patches to disk", "main");
      if (!FreePatchesInMem(patches))
        LOG_ERROR("freeing patches data structure in memory", "main");

      /* Output format can be HDF, GeoTiff, raw binary, or both HDF and
         GeoTiff */
      if (param->output_file_format == HDF_FMT ||
          param->output_file_format == BOTH)
      { /* Output is HDF */
        /* Create output file. If the output has multiple resolutions, then
           the resolution value will be used as an extension to the basename.
           Otherwise no resolution extension will be used. */
        if (param->multires)
        {
          if (param->output_space_def.proj_num != PROJ_GEO)
            /* Output the pixel size with only two decimal places, since
               the pixel size will be in meters */
            sprintf(HDF_File, "%s_%dm.hdf", param->output_file_name,
              (int) param->output_pixel_size[curr_sds]);
          else
            /* Output the pixel size with four decimal places, since the
               pixel size will be in degrees (need to convert from radians) */
            sprintf(HDF_File, "%s_%.04fd.hdf", param->output_file_name,
              param->output_pixel_size[curr_sds] * DEG);
        }
        else
        {
          sprintf(HDF_File, "%s.hdf", param->output_file_name);
        }

        /* Does the HDF file need to be created? */
        if (param->create_output[curr_sds])
        {
          /* Create the output HDF file */
          if (!CreateOutput(HDF_File))
            LOG_ERROR("creating output image file", "main");
          file_created = true;

          /* Loop through the rest of the SDSs and unmark the ones of the same
             resolution, since they will be output to this same HDF file
             and therefore do not need to be recreated. */
          for (k = curr_sds; k < param_save->num_input_sds; k++)
          {
            if (param->output_pixel_size[k] ==
                param->output_pixel_size[curr_sds])
              param_save->create_output[k] = false;
          }
        }

        /* Open output file */
        output = OutputFile(HDF_File, param->output_sds_name,
          param->output_data_type, &param->output_space_def);
        if (output == (Output_t *)NULL)
          LOG_ERROR("opening output HDF file", "main");
      }

      if (param->output_file_format == GEOTIFF_FMT ||
          param->output_file_format == BOTH)
      { /* Output is geotiff */
        /* Attach the SDS name to the output file name */
        if (param->output_file_format == GEOTIFF_FMT)
        {
          output = &output_mem;
          output->size.l = param->output_space_def.img_size.l;
          output->size.s = param->output_space_def.img_size.s;
          output->open   = true;
        }

        /* Open and initialize the GeoTiff File */
        MasterGeoMem = Open_GEOTIFF(param);
        if( ! MasterGeoMem ) {
           LOG_ERROR("allocating GeoTiff file id structure", "main");
        } else if( MasterGeoMem->error ) {
           LOG_ERROR(MasterGeoMem->error_msg, "main");
        }

/* Remove due to clash of tiff and hdf header files.
 *        if (!OpenGeoTIFFFile (param, &MasterGeoMem))
 *         LOG_ERROR("opening and initializing GeoTiff file", "main");
 */
      }

      if (param->output_file_format == RB_FMT)
      { /* Output is raw binary */
        output = &output_mem;
        output->size.l = param->output_space_def.img_size.l;
        output->size.s = param->output_space_def.img_size.s;
        output->open   = true;

        /* Get the size of the data type */
        switch (param->output_data_type)
        {
          case DFNT_INT8:
          case DFNT_UINT8:
            /* one byte in size */
            output->output_dt_size = 1;
            break;

          case DFNT_INT16:
          case DFNT_UINT16:
            /* two bytes in size */
            output->output_dt_size = 2;
            break;

          case DFNT_INT32:
          case DFNT_UINT32:
          case DFNT_FLOAT32:
            /* four bytes in size */
            output->output_dt_size = 4;
            break;
        }

        /* Copy the SDS name and remove any '/'s in the SDSname */
        j = 0;
        for (i = 0; i < (int)strlen(param->output_sds_name); i++)
        {
          if (param->output_sds_name[i] != '/' &&
              param->output_sds_name[i] != '\\')
          {
            sdsname[j++] = param->output_sds_name[i];
          }
          sdsname[j] = '\0';
        }

        /* Remove any spaces (from the SDS name) from the name */
        k = 0;
        while (sdsname[k])
        {
          if (isspace(sdsname[k]))
            sdsname[k] = '_';
          k++;
        }

        /* Add the SDS band name and dat extension to the filename */
        sprintf(filename, "%s_%s.dat", param->output_file_name,
          sdsname);

        rbfile = fopen(filename, "wb");
        if (rbfile == NULL)
          LOG_ERROR("opening output raw binary file", "main");
      }

      /* Read patches (in input data type) and write to output file (in
         output data type). If NN kernel, then fill any holes left from the
         resampling process. */
      if (!UnscramblePatches(patches, output, param->output_file_format,
          MasterGeoMem, rbfile, param->output_data_type, param->kernel_type))
        LOG_ERROR("unscrambling the output file", "main");

      /* Done with the patches */
      if (!FreePatches(patches))
        LOG_ERROR("freeing patches", "main");

      /* Close output HDF file */
      if (param->output_file_format == HDF_FMT ||
          param->output_file_format == BOTH)
      {
        if (!CloseOutput(output))
          LOG_ERROR("closing output file", "main");

        /* If not appending, write metadata to output HDF file */
        if (file_created)
        {
          if (!WriteMeta(output->file_name, &param->output_space_def)) 
            LOG_ERROR("writing metadata", "main");
        }
      }

      /* Close output GeoTiff file */
      if (param->output_file_format == GEOTIFF_FMT ||
          param->output_file_format == BOTH)
      {
        Close_GEOTIFF( MasterGeoMem );
        /* CloseGeoTIFFFile(&MasterGeoMem); */
        output->open = false;
      }

      /* Close output raw binary file */
      if (param->output_file_format == RB_FMT)
      {
        fclose(rbfile);
        output->open = false;
      }

      /* Free remaining memory */
      if (!FreeGeoloc(geoloc)) 
        LOG_ERROR("freeing geoloc file stucture", "main");
      if (!FreeInput(input)) 
        LOG_ERROR("freeing input file stucture", "main");
      if (param->output_file_format == HDF_FMT ||
          param->output_file_format == BOTH) {
        if (!FreeOutput(output)) 
          LOG_ERROR("freeing output file stucture", "main");
      }

      /* Get rid of patches file */
      ThisPid = getpid();
      sprintf(CharThisPid,"%d",(int)ThisPid);
      strcpy(FinalFileName,param->patches_file_name);
      strcat(FinalFileName,CharThisPid);

      exec_resamp = remove(FinalFileName);
      if(exec_resamp == -1)
      {
        LOG_ERROR("Something bad happened deleting patches file", "main");
      }  

      /* Free the parameter structure */
      if (!FreeParam(param)) 
        LOG_ERROR("freeing user parameter structure", "main");
    } /* loop through bands in the current SDS */
  } /* loop through SDSs */

  /* If output format is HDF then append the metadata, for all resolutions */
  if (param_save->output_file_format == HDF_FMT ||
      param_save->output_file_format == BOTH)
  {
    /* Initialize the create_output structure again for all the SDSs. This
       will be used to determine if the metadata needs to be appended. */
    for (curr_sds = 0; curr_sds < param_save->num_input_sds; curr_sds++)
      param_save->create_output[curr_sds] = true;

    /* Loop through all the SDSs */
    for (curr_sds = 0; curr_sds < param_save->num_input_sds; curr_sds++)
    {
      /* Do we need to append the metadata or has it already been done for
         the current SDSs HDF file? */
      if (!param_save->create_output[curr_sds])
        continue;

      /* Determine the name of the output HDF file */
      if (param_save->multires)
      {
        if (param_save->output_space_def.proj_num != PROJ_GEO)
          /* Output the pixel size with only two decimal places, since
             the pixel size will be in meters */
          sprintf(HDF_File, "%s_%dm.hdf", param_save->output_file_name,
            (int) param_save->output_pixel_size[curr_sds]);
        else
          /* Output the pixel size with four decimal places, since the
             pixel size will be in degrees (need to convert from radians) */
          sprintf(HDF_File, "%s_%.04fd.hdf", param_save->output_file_name,
            param_save->output_pixel_size[curr_sds] * DEG);
      }
      else
      {
        sprintf(HDF_File, "%s.hdf", param_save->output_file_name);
      }

      /* Append the metadata for this HDF file, only for the SDSs of the
         current resolution */
      if (!AppendMetadata(param_save, HDF_File, param_save->input_file_name,
        curr_sds)) 
      {
          /* NOTE: We won't flag this as an error, since in some cases the
             resolution file may not exist.  For example, if a MOD02HKM is
             specified and the Latitude or Longitude data is specified, then
             that data is at a different resolution (1000m) than the rest of
             the image SDS data (500m).  The software will think that there
             should be a 1000m product, however the Latitude and Longitude
             data didn't actually get processed .... since it is float data.
             So, AppendMetadata will flag an error since that HDF file will
             not really exist. */
/*        LOG_ERROR("appending metadata to the output HDF file","main"); */
      }

      /* Loop through the rest of the SDSs and unmark the ones of the same
         resolution, since they will be output to this same HDF file
         and therefore do not need to be recreated. */
      for (k = curr_sds; k < param_save->num_input_sds; k++)
      {
        if (param_save->output_pixel_size[k] ==
            param_save->output_pixel_size[curr_sds])
          param_save->create_output[k] = false;
      }
    }
  }

  /* Free the saved parameter structure */
  if (!FreeParam(param_save))
    LOG_ERROR("freeing saved user parameter structure", "main");

  /* Stop timer and print elapsed time */
  enddate = time(NULL);
  sprintf(msg, "\nEnd Time:  %s", ctime(&enddate));
  LogInfomsg(msg);
  LogInfomsg("Finished processing!\n");
  LogInfomsg( 
    "*********************************************************************"
    "*********\n");

  /* Close the log file */
  CloseLogHandler();

  /* All done */
  exit (EXIT_SUCCESS);
}
