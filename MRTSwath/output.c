/*
!C****************************************************************************

!File: output.c
  
!Description: Functions creating and writting data to the HDF output file.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.5 2002/12/02
 Gail Schmidt
 Added support for INT8 data types.

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
  
 ! Design Notes:
   1. The following public functions handle the HDF output files:

        CreateOutput - Create new output file.
	OutputFile - Setup 'output' data structure.
	CloseOutput - Close the output file.
	FreeOutput - Free the 'output' data structure memory.
	WriteOutput - Write a line of data to the output product file.

   2. 'OutputFile' must be called before any of the other routines (except for 
      'CreateOutput').  
   3. 'FreeOutput' should be used to free the 'output' data structure.
   4. The only output file type supported is HDF.

!END****************************************************************************
*/

#include <stdlib.h>
#include "output.h"
#include "myerror.h"
#include "myproj.h"
#include "const.h"

bool CreateOutput(char *file_name)
/* 
!C******************************************************************************

!Description: 'CreateOuptut' creates a new HDF output file.
 
!Input Parameters:
 file_name      output file name

!Output Parameters:
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the creation of the output file failes.
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. The output file is in HDF format and closed after it is created.

!END****************************************************************************
*/
{
  int32 hdf_file_id;

  /* Create the file with HDF open */

  hdf_file_id = Hopen(file_name, DFACC_CREATE, DEF_NDDS); 
  if(hdf_file_id == HDF_ERROR) {
    LOG_RETURN_ERROR("creating output file", "CreateOutput", false); 
  }

  /* Close the file */

  Hclose(hdf_file_id);

  return true;
}

Output_t *OutputFile(char *file_name, char *sds_name, 
                     int output_data_type, Space_def_t *space_def)
/* 
!C******************************************************************************

!Description: 'OutputFile' sets up the 'output' data structure, opens the
 output file for write access, and creates the output Science Data Set (SDS).
 
!Input Parameters:
 file_name      output file name
 sds_name       name of sds to be created
 output_data_type  output HDF data type; data types currently supported are
                     CHAR8, UINT8, INT8, INT16, UINT16, INT32, and UINT32.
 space_def      output space definition; the following fields are input:
                   img_size

!Output Parameters:
 (returns)      'output' data structure or NULL when an error occurs

!Team Unique Header:

 ! Design Notes:
   1. When 'OutputFile' returns sucessfully, the file is open for HDF access 
      and the SDS is open for access.
   2. An error status is returned when:
       a. the output image dimensions are zero or negative
       b. an invalid output data type is passed
       c. memory allocation is not successful
       d. duplicating strings is not successful
       e. errors occur when opening the output HDF file
       f. errors occur when creating and initializing the SDS.
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   4. 'FreeOutput' should be called to deallocate memory used by the 
      'output' data structures.
   5. 'CloseFile' should be called after all of the data is written and 
      before the 'output' data structure memory is released.

!END****************************************************************************
*/
{
  Output_t *this;
  int ir;
  char *error_string = (char *)NULL;
  char tmpstr[1024];

  /* Check parameters */
  
  if (space_def->img_size.l < 1)
    LOG_RETURN_ERROR("invalid number of output lines", 
                 "OutputFile", (Output_t *)NULL);

  if (space_def->img_size.s < 1)
    LOG_RETURN_ERROR("invalid number of samples per output line", 
                 "OutputFile", (Output_t *)NULL);

  if (output_data_type != DFNT_CHAR8  &&
      output_data_type != DFNT_UINT8  &&
      output_data_type != DFNT_INT8  &&
      output_data_type != DFNT_INT16  &&
      output_data_type != DFNT_UINT16 &&
      output_data_type != DFNT_INT32  &&
      output_data_type != DFNT_UINT32)
    LOG_RETURN_ERROR("output data type not supported", "OpenOutput", 
                 (Output_t *)NULL);

  /* Create the Output data structure */

  this = (Output_t *)malloc(sizeof(Output_t));
  if (this == (Output_t *)NULL) 
    LOG_RETURN_ERROR("allocating Output data structure", "OpenOutput", 
                 (Output_t *)NULL);

  /* Populate the data structure */

  this->file_name = DupString(file_name);
  if (this->file_name == (char *)NULL) {
    free(this);
    LOG_RETURN_ERROR("duplicating file name", "OutputFile", (Output_t *)NULL);
  }

  this->sds.name = DupString(sds_name);
  if (this->sds.name == (char *)NULL) {
    free(this->file_name);
    free(this);
    LOG_RETURN_ERROR("duplicating sds name", "OutputFile", (Output_t *)NULL);
  }

  this->size.l = space_def->img_size.l;
  this->size.s = space_def->img_size.s;

  /* Open file for SD access */

  this->sds_file_id = SDstart((char *)file_name, DFACC_RDWR);
  if (this->sds_file_id == HDF_ERROR) {
    free(this->sds.name);
    free(this->file_name);
    free(this);  
    LOG_RETURN_ERROR("opening output file for SD access", "OutputFile", 
                 (Output_t *)NULL); 
  }
  this->open = true;

  /* Set up SDS */

  this->sds.type = output_data_type;
  this->sds.rank = 2;
  this->sds.dim[0].nval = this->size.l;
  this->sds.dim[1].nval = this->size.s;
  if (!PutSDSInfo(this->sds_file_id, &this->sds))
    error_string = "setting up the SDS";

  if (error_string == (char *)NULL) {
    this->sds.dim[0].type = output_data_type;
    this->sds.dim[1].type = output_data_type;

    if (space_def->proj_num == PROJ_GEO)
       sprintf (tmpstr, "lines %.8f", space_def->pixel_size*DEG);
    else
       sprintf (tmpstr, "lines %.2f", space_def->pixel_size);

    this->sds.dim[0].name = DupString(tmpstr);
    if (this->sds.dim[0].name == (char *)NULL) {
      SDendaccess(this->sds.id);
      error_string = "duplicating dim name (l)";
    }
  }

  if (error_string == (char *)NULL) {
    if (space_def->proj_num == PROJ_GEO)
       sprintf (tmpstr, "samps %.8f", space_def->pixel_size*DEG);
    else
       sprintf (tmpstr, "samps %.2f", space_def->pixel_size);

    this->sds.dim[1].name = DupString(tmpstr);
    if (this->sds.dim[1].name == (char *)NULL) {
      free(this->sds.dim[0].name);
      SDendaccess(this->sds.id);
      error_string = "duplicating dim name (s)";
    }
  }

  if (error_string == (char *)NULL) {
    for (ir = 0; ir < this->sds.rank; ir++) {
      if (!PutSDSDimInfo(this->sds.id, &this->sds.dim[ir], ir)) {
        free(this->sds.dim[1].name);
        free(this->sds.dim[0].name);
        SDendaccess(this->sds.id);
        error_string = "setting up the SDS";
	break; 
      }
    }
  }

  if (error_string != (char *)NULL) {
    SDend(this->sds_file_id);
    free(this->sds.name);
    free(this->file_name);
    free(this);  
    LOG_RETURN_ERROR(error_string, "OutputFile", 
                 (Output_t *)NULL); 
  }

  return this;
}


bool CloseOutput(Output_t *this)
/* 
!C******************************************************************************

!Description: 'CloseOutput' ends SDS access and closes the output file.
 
!Input Parameters:
 this           'output' data structure; the following fields are input:
                   open, sds.id, sds_file_id

!Output Parameters:
 this           'output' data structure; the following fields are modified:
                   open
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the file is not open for access
       b. an error occurs when closing access to the SDS.
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'OutputFile' must be called before this routine is called.
   4. 'FreeOutput' should be called to deallocate memory used by the 
      'output' data structures.

!END****************************************************************************
*/
{

 if (!this->open)
    LOG_RETURN_ERROR("file not open", "CloseOutput", false);

  if (SDendaccess(this->sds.id) == HDF_ERROR) 
    LOG_RETURN_ERROR("ending sds access", "CloseOutput", false);

  SDend(this->sds_file_id);
  this->open = false;

  return true;
}


bool FreeOutput(Output_t *this)
/* 
!C******************************************************************************

!Description: 'FreeOutput' frees the 'output' data structure memory.
 
!Input Parameters:
 this           'output' data structure; the following fields are input:
                   sds.rank, sds.dim[*].name, sds.name, file_name

!Output Parameters:
 this           'output' data structure; the following fields are modified:
                   sds.dim[*].name, sds.name, file_name
 (returns)      status:
                  'true' = okay (always returned)

!Team Unique Header:

 ! Design Notes:
   1. 'OutputFile' must be called before this routine is called.
   2. An error status is never returned.

!END****************************************************************************
*/
{
  int ir;

  if (this != (Output_t *)NULL) {
    for (ir = 0; ir < this->sds.rank; ir++) {
      if (this->sds.dim[ir].name != (char *)NULL) 
        free(this->sds.dim[ir].name);
    }
    if (this->sds.name != (char *)NULL) free(this->sds.name);
    if (this->file_name != (char *)NULL) free(this->file_name);
    free(this);
  }

  return true;
}

bool WriteOutput(Output_t *this, int iline, void *buf)
/* 
!C******************************************************************************

!Description: 'WriteOutput' writes a line of data to the output HDF file.
 
!Input Parameters:
 this           'output' data structure; the following fields are input:
                   open, size, sds.id
 iline          output line number
 buf            buffer of data to be written

!Output Parameters:
 this           'output' data structure; the following fields are modified:
 (returns)      status:
                  'true' = okay
		  'false' = error return

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned when:
       a. the file is not open for access
       b. the line number is invalid (< 0; >= 'this->size.l')
       b. an error occurs when writting to the SDS.
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. 'OutputFile' must be called before this routine is called.

!END****************************************************************************
*/
{
  int32 start[MYHDF_MAX_RANK], nval[MYHDF_MAX_RANK];

  /* Check the parameters */

  if (!this->open)
    LOG_RETURN_ERROR("file not open", "WriteOutput", false);

  if (iline < 0  ||  iline >= this->size.l)
    LOG_RETURN_ERROR("invalid line number", "WriteOutput", false);

  /* Write the data */

  start[0] = iline;
  start[1] = 0;
  nval[0] = 1;
  nval[1] = this->size.s;

  if (SDwritedata(this->sds.id, start, NULL, nval, 
                  buf) == HDF_ERROR)
      LOG_RETURN_ERROR("writing output", "WriteOutput", false);
  
  return true;
}
