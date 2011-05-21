/*
!C****************************************************************************

!File: myhdf.c
  
!Description: Functions for handling HDF files.

!Revision History:
 Revision 1.0 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.5 2002/11/02
 Gail Schmidt
 Added support for INT8 data types.

 Revision 2.0 2003/10/23
 Gail Schmidt
 Added routine to read the bounding coords from the metadata.
 Added support for multiple pixel sizes and number of lines/samples.

 Revision 2.0a 2004/09/23
 Gail Schmidt
 Modified the routine that reads the bounding coords to look for the
 bounding coords in the main metadata section, if it is not found in the
 Struct, Archive, or Core metadata structures.

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
      phone: 301-614-5508               Lanham, MD 20770  
  
 ! Design Notes:
   1. The following functions handle Science Data Sets (SDSs) and attributes
      in HDF files:

       GetSDSInfo - Read SDS information.
       GetSDSDimInfo - Read SDS dimension information.
       PutSDSInfo - Create an SDS and write information.
       PutSDSDimInfo - Write SDS dimension information.
       GetAttrDouble - Get an HDF attribute's value.
       ReadBoundCoords - Read the bounding coordinates from the metadata.
       ReadMetadata - Read the specified attribute from the metadata.
       DetermineResolution - Determine the resolution of the specified SDSs.
       DeterminePixelSize - Determine the pixel size of the specified SDSs.

!END****************************************************************************
*/

#include <stdlib.h>
#include <math.h>
#include "myhdf.h"
#include "myerror.h"
#include "hdf.h"
#include "mfhdf.h"
#include "mystring.h"
#include "input.h"
#include "geoloc.h"
#include "myproj.h"
#include "const.h"

/* Constants */

#define DIM_MAX_NCHAR (80)  /* Maximum size of a dimension name */

/* Functions */


bool GetSDSInfo(int32 sds_file_id, Myhdf_sds_t *sds)
/* 
!C******************************************************************************

!Description: 'GetSDSInfo' reads information for a specific SDS.
 
!Input Parameters:
 sds_file_id    SDS file id

!Output Parameters:
 sds            SDS data structure; the following fields are updated:
                  index, name, id, rank, type, nattr
 (returns)      Status:
                  'true' = okay
		  'false' = error reading the SDS information

!Team Unique Header:

 ! Design Notes:
   1. An error status is returned if the SDS rank is greater than 
      'MYHDF_MAX_RANK'.
   2. On normal returns the SDS is selected for access.
   3. The HDF file is assumed to be open for SD (Science Data) access.
   4. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END****************************************************************************
*/
{
  int32 dims[MYHDF_MAX_RANK];

  sds->index = SDnametoindex(sds_file_id, sds->name);
  if (sds->index == HDF_ERROR)
    LOG_RETURN_ERROR("getting sds index", "GetSDSInfo", false);

  sds->id = SDselect(sds_file_id, sds->index);
  if (sds->id == HDF_ERROR)
    LOG_RETURN_ERROR("getting sds id", "GetSDSInfo", false);

  if (SDgetinfo(sds->id, sds->name, &sds->rank, dims, 
                &sds->type, &sds->nattr) == HDF_ERROR) {
    SDendaccess(sds->id);
    LOG_RETURN_ERROR("getting sds information", "GetSDSInfo", false);
  }

  if (sds->rank > MYHDF_MAX_RANK) {
    SDendaccess(sds->id);
    LOG_RETURN_ERROR("sds rank too large", "GetSDSInfo", false);
  }
  return true;
}


bool GetSDSDimInfo(int32 sds_id, Myhdf_dim_t *dim, int irank)
/* 
!C******************************************************************************

!Description: 'GetSDSDimInfo' reads information for a specific SDS dimension.
 
!Input Parameters:
 sds_id         SDS id

!Output Parameters:
 dim            Dimension data structure; the following fields are updated:
                   id, nval, type, nattr, name
 (returns)      Status:
                  'true' = okay
		  'false' = error reading the dimension information

!Team Unique Header:

 ! Design Notes:
   1. The HDF file is assumed to be open for SD (Science Data) access.
   2. An dimension name of less than 'DIM_MAX_NCHAR' is expected.
   3. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END****************************************************************************
*/
{
  char dim_name[DIM_MAX_NCHAR];

  dim->id = SDgetdimid(sds_id, irank);
  if (dim->id == HDF_ERROR) 
    LOG_RETURN_ERROR("getting dimension id", "GetSDSDimInfo", false);

  if (SDdiminfo(dim->id, dim_name,
                &dim->nval, &dim->type, 
	        &dim->nattr) == HDF_ERROR)
      LOG_RETURN_ERROR("getting dimension information", "GetSDSDimInfo",
                            false);

  dim->name = DupString(dim_name);
  if (dim->name == (char *)NULL)
    LOG_RETURN_ERROR("copying dimension name", "GetSDSDimInfo", false);

  return true;
}

bool PutSDSInfo(int32 sds_file_id, Myhdf_sds_t *sds)
/* 
!C******************************************************************************

!Description: 'PutSDSInfo' creates a SDS and writes SDS information.
 
!Input Parameters:
 sds_file_id    SDS file id
 sds            SDS data structure; the following are used:
                   rank, dims[*].nval, name, type

!Output Parameters:
 sds            SDS data structure; the following are updated:
                   id, index
 (returns)      Status:
                  'true' = okay
		  'false' = error writing the SDS information

!Team Unique Header:

 ! Design Notes:
   1. A maximum of 'MYHDF_MAX_RANK' dimensions are expected.
   2. On normal returns the SDS is selected for access.
   3. The HDF file is assumed to be open for SD (Science Data) access.
   4. Error messages are handled with the 'LOG_RETURN_ERROR' macro.

!END****************************************************************************
*/
{
  int irank;
  int32 dims[MYHDF_MAX_RANK];

  for (irank = 0; irank < sds->rank; irank++)
    dims[irank] = sds->dim[irank].nval;

  /* Create the SDS */

  sds->id = SDcreate(sds_file_id, sds->name, sds->type, 
                     sds->rank, dims);
  if (sds->id == HDF_ERROR)
    LOG_RETURN_ERROR("Creating sds", "PutSDSInfo", false);

  sds->index = SDnametoindex(sds_file_id, sds->name);
  if (sds->index == HDF_ERROR)
    LOG_RETURN_ERROR("Getting sds index", "PutSDSInfo", false);

  return true;
}


bool PutSDSDimInfo(int32 sds_id, Myhdf_dim_t *dim, int irank)
/* 
!C******************************************************************************

!Description: 'PutSDSDimInfo' writes information for a SDS dimension.
 
!Input Parameters:
 sds_id         SDS id
 dim            Dimension data structure; the following field is used:
                   name
 irank          Dimension rank

!Output Parameters:
 dim            Dimension data structure; the following field is updated:
                   id
 (returns)      Status:
                  'true' = okay
		  'false' = error writing the dimension information

!Team Unique Header:

 ! Design Notes:
   1. The HDF file is assumed to be open for SD (Science Data) access.
   2. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
   3. Setting the type of the dimension is not currently implemented.

!END****************************************************************************
*/
{

  dim->id = SDgetdimid(sds_id, irank);
  if (dim->id == HDF_ERROR) 
    LOG_RETURN_ERROR("getting dimension id", "PutSDSDimInfo", false);

  if (SDsetdimname(dim->id, dim->name) == HDF_ERROR)
    LOG_RETURN_ERROR("setting dimension name", "PutSDSDimInfo", false);

  /* Set dimension type */

    /* !! do it !! */
 
  return true;
}


bool GetAttrDouble(int32 sds_id, Myhdf_attr_t *attr, double *val)
/* 
!C******************************************************************************

!Description: 'GetAttrDouble' reads an attribute into a parameter of type
 'double'.
 
!Input Parameters:
 sds_id         SDS id
 attr           Attribute data structure; the following field is used:
                   name

!Output Parameters:
 attr           Attribute data structure; the following field is updated:
                   id, type, nval
 val            An array of values from the HDF attribute (converted from the
                  native type to type 'double'.
 (returns)      Status:
                  'true' = okay
		  'false' = error reading the attribute information

!Team Unique Header:

 ! Design Notes:
   1. The values in the attribute are converted from the stored type to 
      'double' type.
   2. The HDF file is assumed to be open for SD (Science Data) access.
   3. If the attribute has more than 'MYHDF_MAX_NATTR_VAL' values, an error
      status is returned.
   4. Error messages are handled with the 'LOG_RETURN_ERROR' macro.
!END****************************************************************************
*/
{
  char8 val_char8[MYHDF_MAX_NATTR_VAL];
  uint8 val_uint8[MYHDF_MAX_NATTR_VAL];
  int8 val_int8[MYHDF_MAX_NATTR_VAL];
  int16 val_int16[MYHDF_MAX_NATTR_VAL];
  uint16 val_uint16[MYHDF_MAX_NATTR_VAL];
  int32 val_int32[MYHDF_MAX_NATTR_VAL];
  uint32 val_uint32[MYHDF_MAX_NATTR_VAL];
  float32 val_float32[MYHDF_MAX_NATTR_VAL];
  float32 val_float64[MYHDF_MAX_NATTR_VAL];
  int i;
  char z_name[80];
  char errmsg[M_MSG_LEN+1];
  
  if ((attr->id = SDfindattr(sds_id, attr->name)) == HDF_ERROR)
  {
    sprintf (errmsg, "getting attribute id: %s", attr->name);
    LOG_RETURN_ERROR(errmsg, "ReadAttrDouble", false);
  }
  if (SDattrinfo(sds_id, attr->id, z_name, &attr->type, &attr->nval) == 
      HDF_ERROR)
  {
    sprintf (errmsg, "getting attribute info for %s", attr->name);
    LOG_RETURN_ERROR("getting attribute info", "ReadAttrDouble", false);
  }

  if (attr->nval < 1)
    LOG_RETURN_ERROR("no attribute value", "ReadAttrDouble", false);
  if (attr->nval > MYHDF_MAX_NATTR_VAL) 
    LOG_RETURN_ERROR("too many attribute values", "ReadAttrDouble", false);

  switch (attr->type) {
  case DFNT_CHAR8:
    if (SDreadattr(sds_id, attr->id, val_char8) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (char8)","ReadAttrDouble",false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_char8[i];
    break;
  case DFNT_UINT8:
    if (SDreadattr(sds_id, attr->id, val_uint8) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (uint8)","ReadAttrDouble",false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_uint8[i];
    break;
  case DFNT_INT8:
    if (SDreadattr(sds_id, attr->id, val_int8) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (int8)", "ReadAttrDouble",false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_int8[i];
    break;
  case DFNT_INT16:
    if (SDreadattr(sds_id, attr->id, val_int16) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (int16)","ReadAttrDouble",false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_int16[i];
    break;
  case DFNT_UINT16:
    if (SDreadattr(sds_id, attr->id, val_uint16) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (uint16)", "ReadAttrDouble", false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_uint16[i];
    break;
  case DFNT_INT32:
    if (SDreadattr(sds_id, attr->id, val_int32) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (int32)", "ReadAttrDouble", false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_int32[i];
    break;
  case DFNT_UINT32:
    if (SDreadattr(sds_id, attr->id, val_uint32) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (uint32)", "ReadAttrDouble", false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_uint32[i];
    break;
  case DFNT_FLOAT32:
    if (SDreadattr(sds_id, attr->id, val_float32) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (float32)", "ReadAttrDouble", false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_float32[i];
    break;
  case DFNT_FLOAT64:
    if (SDreadattr(sds_id, attr->id, val_float64) == HDF_ERROR) 
      LOG_RETURN_ERROR("reading attribute (float64)", "ReadAttrDouble", false);
    for (i = 0; i < attr->nval; i++) 
      val[i] = (double)val_float64[i];
    break;
  }

  return true;
}

/*
!C******************************************************************************
!Description: 'ReadBoundCoords' sends requests to ReadMetadata to read the
    archive, core, and structural metadata from the HDF file.  The
    metadata is read and the bounding rectangular coordinates are searched
    for.  Once these coords are found, then the routine returns.

!Input Parameters:
 char string      HDF filename

!Output Parameters:
 ul_coord         'Geo_coord_t' to hold the UL lat/long
 lr_coord         'Geo_coord_t' to hold the LR lat/long

 (returns)      Status:
                  'true' = okay
                  'false' = error reading the bounding coords

!Team Unique Header:

! Design Notes:
!END*****************************************************************************/
bool ReadBoundCoords
(
    char *infile,             /* I: HDF filename */
    Geo_coord_t *ul_corner,   /* O: UL corner lat/long value */
    Geo_coord_t *lr_corner    /* O: LR corner lat/long value */
)

{
    int j;                   /* looping variable */
    bool status;             /* return status */
    intn sd_status;          /* value returned by SD functions */
    char attrname[256];      /* holds the file_name string */
    double bound_coords[4];  /* array of bounding coords (N, S, E, W) */
    bool coords_present;     /* were all bounding coordinates present? */
    int32 sd_id;             /* SD file id # for the HDF file */
    int32 my_attr_index;     /* holds return val from SDfindattr */
    double tmpdouble;        /* temporary value for reading the bounding
                                coords */

    sd_id = SDstart(infile, DFACC_RDONLY);
    if (sd_id < 0) {
        LOG_RETURN_ERROR("reading input HDF file", "ReadBoundCoords", false);
    }

    /* First, ArchiveMetadata alone */
    status = ReadMetadata(sd_id, "ArchiveMetadata", bound_coords,
        &coords_present);
    if (status && coords_present)
    {
        /* Assign the UL and LR corners */
        ul_corner->lat = bound_coords[0];  /* North */
        ul_corner->lon = bound_coords[2];  /* East */
        lr_corner->lat = bound_coords[1];  /* South */
        lr_corner->lon = bound_coords[3];  /* West */
        return (true);
    }

    /* Try concatenating sequence numbers */
    for (j = 0; j <= 9; j++)
    {
        sprintf(attrname, "%s.%d", "ArchiveMetadata", j);
        status = ReadMetadata(sd_id, attrname, bound_coords, &coords_present);
        if (status && coords_present)
        {
            /* Assign the UL and LR corners */
            ul_corner->lat = bound_coords[0];  /* North */
            ul_corner->lon = bound_coords[2];  /* East */
            lr_corner->lat = bound_coords[1];  /* South */
            lr_corner->lon = bound_coords[3];  /* West */
            return (true);
        }
    }

    /* Now try CoreMetadata alone */
    status = ReadMetadata(sd_id, "CoreMetadata", bound_coords,
        &coords_present);
    if (status && coords_present)
    {
        /* Assign the UL and LR corners */
        ul_corner->lat = bound_coords[0];  /* North */
        ul_corner->lon = bound_coords[2];  /* East */
        lr_corner->lat = bound_coords[1];  /* South */
        lr_corner->lon = bound_coords[3];  /* West */
        return (true);
    }

    /* Try concatenating sequence numbers */
    for (j = 0; j <= 9; j++)
    {
        sprintf(attrname, "%s.%d", "CoreMetadata", j);
        status = ReadMetadata(sd_id, attrname, bound_coords, &coords_present);
        if (status && coords_present)
        {
            /* Assign the UL and LR corners */
            ul_corner->lat = bound_coords[0];  /* North */
            ul_corner->lon = bound_coords[2];  /* East */
            lr_corner->lat = bound_coords[1];  /* South */
            lr_corner->lon = bound_coords[3];  /* West */
            return (true);
        }
    }

    /* Last, try StructMetadata alone */
    status = ReadMetadata(sd_id, "StructMetadata", bound_coords,
        &coords_present);
    if (status && coords_present)
    {
        /* Assign the UL and LR corners */
        ul_corner->lat = bound_coords[0];  /* North */
        ul_corner->lon = bound_coords[2];  /* East */
        lr_corner->lat = bound_coords[1];  /* South */
        lr_corner->lon = bound_coords[3];  /* West */
        return (true);
    }

    /* Try concatenating sequence numbers */
    for (j = 0; j <= 9; j++)
    {
        sprintf(attrname, "%s.%d", "StructMetadata", j);
        status = ReadMetadata(sd_id, attrname, bound_coords, &coords_present);
        if (status && coords_present)
        {
            /* Assign the UL and LR corners */
            ul_corner->lat = bound_coords[0];  /* North */
            ul_corner->lon = bound_coords[2];  /* East */
            lr_corner->lat = bound_coords[1];  /* South */
            lr_corner->lon = bound_coords[3];  /* West */
            return (true);
        }
    }

    /* The bounding coords are not in the Struct, Core, or Archive structures,
       so look to see if they are just lying in the global metadata itself. */
    /* North coordinate */
    my_attr_index = SDfindattr(sd_id, "NORTHBOUNDINGCOORDINATE");
    if (my_attr_index == -1)
        return (false);

    /* Read attribute from the HDF file */
    sd_status = SDreadattr(sd_id, my_attr_index, &tmpdouble);
    if (sd_status == -1)
        return (false);
    ul_corner->lat = tmpdouble;  /* North */

    /* South coordinate */
    my_attr_index = SDfindattr(sd_id, "SOUTHBOUNDINGCOORDINATE");
    if (my_attr_index == -1)
        return (false);

    /* Read attribute from the HDF file */
    sd_status = SDreadattr(sd_id, my_attr_index, &tmpdouble);
    if (sd_status == -1)
        return (false);
    lr_corner->lat = tmpdouble;  /* South */

    /* East coordinate */
    my_attr_index = SDfindattr(sd_id, "EASTBOUNDINGCOORDINATE");
    if (my_attr_index == -1)
        return (false);

    /* Read attribute from the HDF file */
    sd_status = SDreadattr(sd_id, my_attr_index, &tmpdouble);
    if (sd_status == -1)
        return (false);
    ul_corner->lon = tmpdouble;  /* East */

    /* West coordinate */
    my_attr_index = SDfindattr(sd_id, "WESTBOUNDINGCOORDINATE");
    if (my_attr_index == -1)
        return (false);

    /* Read attribute from the HDF file */
    sd_status = SDreadattr(sd_id, my_attr_index, &tmpdouble);
    if (sd_status == -1)
        return (false);
    lr_corner->lon = tmpdouble;  /* West */

    /* Return the value of the function to main function */
    return (true);
}


/*
!C******************************************************************************
!Description: 'ReadMetadata' reads the user-defined metadata from the HDF
              file and determines the bounding rectangular coordinates.

!Input Parameters:
 sd_id           SD id for the HDF file
 attr            Attribute to read

!Output Parameters:
 bound_coords   Array of the bounding coords (north, south, east, west)
 coords_present Boolean value to specify if all the bounding coords are
                  present
 (returns)      Status:
                  'true' = okay
                  'false' = error reading the bounding coords

!Team Unique Header:

! Design Notes:
!END*****************************************************************************/
bool ReadMetadata
(
    int32 sd_id,             /* I: the SD id # for the HDF file */
    char *attr,              /* I: the name handle of the attribute to read */
    double bound_coords[],   /* O: bounding coords (N, S, E, W) */
    bool *coords_present     /* O: were all bounding coordinates present? */
)

{
    intn status;            /* value returned by SD functions */
    bool retstatus;         /* this is the var that holds return val */
    int32 my_attr_index;    /* holds return val from SDfindattr */
    int32 data_type;        /* holds attribute's data type */
    int32 n_values;         /* stores # of vals of the attribute */
    int num_chars;          /* number of characters read in the line */
    char *file_data = NULL; /* char ptr used to allocate temp space during
                               transfer of attribute info */
    char *file_data_ptr = NULL;
                            /* pointer to file_data for scanning */
    char attr_name[MAX_NC_NAME];  /* holds attributes name */
    char token_buffer[256]; /* holds the current token */
    bool found_north_bound; /* north bounding rectangle coord been found? */
    bool found_south_bound; /* south bounding rectangle coord been found? */
    bool found_east_bound;  /* east bounding rectangle coord been found? */
    bool found_west_bound;  /* west bounding rectangle coord been found? */

    /* look for attribute in the HDF file */
    my_attr_index = SDfindattr(sd_id, attr);

    /* only proceed if the attribute was found */
    if (my_attr_index == -1)
        return (false);

    /* get size of HDF file attribute */
    status = SDattrinfo(sd_id, my_attr_index, attr_name, &data_type,
        &n_values);
    if (status == -1)
        return (false);

    /* attempt to allocate memory for HDF file attribute contents (add one
       character for the end of string character) */
    file_data = (char *) calloc (n_values+1, sizeof(char));
    if (file_data == NULL)
        LOG_RETURN_ERROR("unable to allocate memory", "ReadMetadata",
                              false);

    /* read attribute from the HDF file */
    status = SDreadattr(sd_id, my_attr_index, file_data);
    if (status == -1)
    {
        /* first free the allocated memory */
        free (file_data);

        return (false);
    }

    /* walk through the file_data string one token at a time looking for the
       bounding rectangular coords */
    found_north_bound = false;
    found_south_bound = false;
    found_east_bound = false;
    found_west_bound = false;
    file_data_ptr = file_data;
    retstatus = false;
    while (sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars) != EOF)
    {
        /* if this token is END, then we are done with the metadata */
        if (!strcmp (token_buffer, "END"))
            break;

        /* if all the bounding coords have been found, don't waste time
           with the rest of the metadata */
        if (found_north_bound && found_south_bound && found_east_bound &&
            found_west_bound)
        {
            break;
        }

        /* increment the file_data_ptr pointer to point to the next token */
        file_data_ptr += num_chars;

        /* look for the NORTHBOUNDINGCOORDINATE token */
        if (!found_north_bound &&
            !strcmp (token_buffer, "NORTHBOUNDINGCOORDINATE"))
        {
            /* read the next three tokens, this should be the
               NUM_VAL = ... line */
            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            /* read the next three tokens, this should be the
               VALUE = ... line */
            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            /* find the value of the bounding coordinate */
            bound_coords[0] = atof (token_buffer);
            found_north_bound = true;
            retstatus = true;
            continue;  /* don't waste time looking for the other
                          bounding rectangles with this token */
        }

        /* look for the SOUTHBOUNDINGCOORDINATE token */
        if (!found_south_bound &&
            !strcmp (token_buffer, "SOUTHBOUNDINGCOORDINATE"))
        {
            /* read the next three tokens, this should be the
               NUM_VAL = ... line */
            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            /* read the next three tokens, this should be the
               VALUE = ... line */
            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            /* find the value of the bounding coordinate */
            bound_coords[1] = atof (token_buffer);
            found_south_bound = true;
            retstatus = true;
            continue;  /* don't waste time looking for the other
                          bounding rectangles with this token */
        }

        /* look for the EASTBOUNDINGCOORDINATE token */
        if (!found_east_bound &&
            !strcmp (token_buffer, "EASTBOUNDINGCOORDINATE"))
        {
            /* read the next three tokens, this should be the
               NUM_VAL = ... line */
            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            /* read the next three tokens, this should be the
               VALUE = ... line */
            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            /* find the value of the bounding coordinate */
            bound_coords[2] = atof (token_buffer);
            found_east_bound = true;
            retstatus = true;
            continue;  /* don't waste time looking for the other
                          bounding rectangles with this token */
        }

        /* look for the WESTBOUNDINGCOORDINATE token */
        if (!found_west_bound &&
            !strcmp (token_buffer, "WESTBOUNDINGCOORDINATE"))
        {
            /* read the next three tokens, this should be the
               NUM_VAL = ... line */
            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            /* read the next three tokens, this should be the
               VALUE = ... line */
            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            sscanf (file_data_ptr, "%s%n", token_buffer, &num_chars);
            file_data_ptr += num_chars;

            /* find the value of the bounding coordinate */
            bound_coords[3] = atof (token_buffer);
            found_west_bound = true;
            retstatus = true;
            continue;  /* don't waste time looking for the other
                          bounding rectangles with this token */
        }
    }

    /* did we read all the bounding coordinates? */
    if (found_north_bound && found_south_bound && found_east_bound &&
        found_west_bound)
        *coords_present = true;
    else
        *coords_present = false;

    /* free dynamically allocated memory */
    free (file_data);

    /* if any of the bounding coords were found, then true is returned.
       otherwise return false */
    return (retstatus);
}


bool DetermineResolution(Myhdf_sds_t *sds, Img_coord_int_t *ls_dim, int *ires)
/*
!C*****************************************************************************
!Description: 'DetermineResolution' determines the resolution of the SDS,
 based on the nominal 1KM frame size.

!Input Parameters:
 sds            SDS structure. The dimension values are used to determine
                the number of lines in the SDS.
 ls_dim         Dimension locations for the line and sample dimensions.

!Output Parameters:
 ires           Resolution value of the SDS
 (returns)      Status:
                  'true' = okay
                  'false' = error reading the bounding coords

!Team Unique Header:

! Design Notes:

!END*****************************************************************************/
{
  Img_coord_int_t size;                /* input file size */

  /* Check the line and sample dimensions */
  size.s = sds->dim[ls_dim->s].nval;
  *ires = -1;
  *ires = (int)((size.s / (double)NFRAME_1KM_MODIS) + 0.5);

  /* Verify the resolution. If not 250m, 500m, or 1km product, then this
     is an error. */
  if (*ires != 1 && *ires != 2 && *ires != 4) {
    LOG_RETURN_ERROR("invalid resolution", "DetermineResolution", false);
  }

  return true;
}


bool DeterminePixelSize(char *geoloc_file_name, int num_input_sds, 
  int ires[MAX_SDS_DIMS], int out_proj_num,
  double output_pixel_size[MAX_SDS_DIMS])
/*
!C*****************************************************************************
!Description: 'DeterminePixelSize' uses the ires values (calculated by
 DetermineResolution) to determine the output pixel size.

!Input Parameters:
 Param_t        Input parameter structure. The output_pixel_size values
                are updated for each SDS specified.

!Output Parameters:
 (returns)      Status:
                  'true' = okay
                  'false' = error reading the bounding coords

!Team Unique Header:

! Design Notes:
  1. If the output projection is Geographic, then the pixel size will need
     to be computed in degrees.  The algorithm used in this routine uses the
     nominal pixel size to determine the pixel size in meters.  For Geographic,
     the difference between two lat/long locations will be used to determine
     the pixel size in degrees.  That value will be modified depending on
     whether the resolution is 250m, 500m, or 1km (no modification necessary).

!END*****************************************************************************/
{
  int i;
  Geoloc_t *geoloc = NULL;             /* geolocation file */
  int center_loc;                      /* center location in the scan */
  int midscan;                         /* middle scan location */
  int32 start[MYHDF_MAX_RANK];         /* start reading at this location */
  int32 nval[MYHDF_MAX_RANK];          /* read this many values */
  double center = 0.0;                 /* longitude value at the center */
  double centerp1 = 0.0;               /* longitude value at the center+1 */

  /* If dealing with an output projection of Geographic, then read the
     center pixel values from the Geolocation file for use later in the
     pixel size calculation. */
  if (out_proj_num == PROJ_GEO)
  {
      /* Open geoloc file */
      geoloc = OpenGeolocSwath(geoloc_file_name);
      if (geoloc == (Geoloc_t *)NULL)
        LOG_RETURN_ERROR("bad geolocation file", "DeterminePixelSize",
                              false);

      /* Grab the line representing the center of the swath (nscan/2) */
      midscan = geoloc->nscan / 2;
      start[0] = midscan * geoloc->scan_size.l;
      start[1] = 0;
      nval[0] = 1;
      nval[1] = geoloc->scan_size.s;

      /* Read the longitude values for the entire center scan */
      if (SDreaddata(geoloc->sds_lon.id, start, NULL, nval,
        geoloc->lon_buf) == HDF_ERROR)
        LOG_RETURN_ERROR("reading longitude", "DeterminePixelSize", false);

      /* Get the center of the scan and the center of the scan plus one */
      center_loc = geoloc->scan_size.s / 2;
      center = geoloc->lon_buf[center_loc];
      centerp1 = geoloc->lon_buf[center_loc + 1];

      /* Close geolocation file */
      if (!CloseGeoloc(geoloc))
        LOG_RETURN_ERROR("closing geolocation file", "DeterminePixelSize",
                              false);
  }

  /* Loop through all the SDSs to be processed */
  for (i = 0; i < num_input_sds; i++)
  {
    /* Determine the output pixel size */
    /* If the output projection is not Geographic, then the output pixel
       size is in meters */
    if (out_proj_num != PROJ_GEO)
    {
      switch (ires[i]) {
        case -1:
          /* this SDS won't be processed so set to 1000.0 */
          output_pixel_size[i] = 1000.0;
          break;

        case 1:
          output_pixel_size[i] = 1000.0;
          break;

        case 2:
          output_pixel_size[i] = 500.0;
          break;

        case 4:
          output_pixel_size[i] = 250.0;
          break;

        default:
          /* this SDS won't be processed so set to 1000.0 */
          output_pixel_size[i] = 1000.0;
          break;
/*          LOG_RETURN_ERROR("invalid resolution", "DeterminePixelSize",
                                  false); */
      }
    }

    /* If the output projection is Geographic, then we need to read the
       Geolocation file */
    else
    {
      /* Determine the output pixel size. The geolocation file is at the
         1km resolution, so take into account the actual resolution of this
         SDS (ires value). If the ires is -1, then the SDS won't be
         processed so compute it as if the ires were 1 (1000 m). */
      if (ires[i] == -1)
        output_pixel_size[i] = fabs(centerp1 - center);
      else
        output_pixel_size[i] = fabs(centerp1 - center) / ires[i];
    }
  }

  if (out_proj_num == PROJ_GEO)
  {
    /* Free geolocation structure */
    if (!FreeGeoloc(geoloc))
      LOG_RETURN_ERROR("freeing geoloc file struct", "DeterminePixelSize",
      false);
  }

  return true;
}
