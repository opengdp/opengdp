/*
!C****************************************************************************

!File: usage.h

!Description: Usage and help information for the MODIS 'swath2grid' program.

!Revision History:
 Revision 1.0 2000/12/13
 Robert Wolfe
 Original Version.

!Revision History:
 Revision 1.1 2001/05/08
 Robert Wolfe
 Original Version.

 Revision 1.2 2002/05/10
 Robert Wolfe
 Added usage information for parameter file.

 Revision 1.5 2002/12/02
 Gail Schmidt
 Added support for INT8 data types.

 Revision 2.0 2003/11
 Gail Schmidt
 Removed support for input proj information, since GRIDs won't be
   supported for MRTSwath. Also added support for specifying multiple
 SDSs and no SDSs (default is to process all SWATHs in the HDF file).
 Added support for multiple pixel sizes.
 Allow the user to specify the UL/LR corner in line/sample units.
 Removed the append option, since it is automatically used for HDF files.
 

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

      Sadashiva Devadiga (Code 922)
      MODIS Land Team Support Group     SSAI
      devadiga@ltpmail.gsfc.nasa.gov    5900 Princess Garden Pkway, #300
      phone: 301-614-5549               Lanham, MD 20706

!END****************************************************************************
*/

#ifndef USAGE_H
#define USAGE_H

#define HELP \
" \n" \
"     swath2grid - resample one or more SDS from an HDF file \n" \
" \n" \
"SYNOPSIS \n" \
"    swath2grid -if=<input file> -of=<output file> -gf=<geolocation file>\n" \
"           [-off=<output file format (HDF_FMT, GEOTIFF_FMT, RB_FMT)>]\n" \
"           [-sds=<SDS name>] [-kk=<resampling type (NN, BI, CC)>]\n" \
"           -oproj=<output projection>\n" \
"           [-oprm=<output projection parameters>]\n" \
"           [-opsz=<output pixel sizes>]\n" \
"           [-oul=<output upper left corner>]\n" \
"           [-olr=<output lower right corner>]\n" \
"           [-ozn=<output zone number>]\n" \
"           [-osst=<output spatial subset type\n" \
"                   (LAT_LONG, PROJ_COORDS, LINE_SAMPLE)>]\n" \
"           [-osp=<output sphere number>]\n" \
"           [-oty=<output data type>]\n" \
"           [-pf=<parameter file>]\n" \
" \n" \
"DESCRIPTION \n" \
"    Resample one or more SDSs from a L2 MODIS granule into user-specified\n"\
"    projection coordinates.  The required 2D data slice in a 3D SDS is\n" \
"    specified by including the number (0-based) identifying the slice in\n" \
"    the SDS name. i.e. to specify 3rd band in the 3D SDS EV_500_RefSB of a\n"\
"    MOD02HKM granule use -sds=EV_500_RefSB,0,0,1. Multiple SDSs can also\n" \
"    be projected. \n" \
"    i.e. EV_1KM_RefSB,0,1,0,1; EV_500_Aggr1km_RefSB,1; EV_250_Aggr1km_RefSB\n"\
"    Output file types include HDF, GeoTiff, and Raw Binary. Multiple SDSs\n" \
"    will be written to the same HDF file, however separate files will\n" \
"    created for multiple SDSs/bands being output to GeoTiff or Raw Binary\n" \
"    files.  In addition, for multiple output resolutions, SDSs with the\n" \
"    same output resolution will be written to the same HDF file. The output\n"\
"    resolution will be appended to the basename if multiple output\n" \
"    resolutions are specified.\n" \
"    i.e mod02hkm_1000m.hdf, mod02hkm_500m.hdf, mod02hkm_250m.hdf\n" \
" \n" \
"OPTIONS \n" \
"    -help                      Display this help message \n" \
"    -help=proj                 Display general help on map projections \n" \
"                               and spheres \n" \
"    -help=param                Display help on parameter files \n" \
"    -help=projection           Display help for a specific map projection \n" \
"                               (number or short name)\n" \
"    -if=input filename         Input L1 or L2 (swath) granule \n" \
"    -of=output filename        Output filename \n" \
"                               No file extension is needed since the output\n"\
"                               file format will specify the extension.\n" \
"                               If an extension of .hdf, .hdr, or .tif is\n" \
"                               specified, then it will be ignored.\n" \
"    -off=output file format    Output file format (HDF_FMT, GEOTIFF_FMT,\n" \
"                               RB_FMT). Default is HDF_FMT.\n" \
"    -gf=geoloc filename        Associated geolocation file for the input\n" \
"                               file\n" \
"    -sds=SDS name              Name of SDS to resample (use quotes for more\n"\
"                               than one specified SDS)\n" \
"                               Use the index number (0-based) after the SDS\n"\
"                               name to identify 2D slice of 3D SDS. \n" \
"                               e.g. -sds=Cloud_Mask,1 for the first band of\n"\
"                               SDS Cloud_Mask in MOD35 granule. \n" \
"                               e.g. EV_1KM_RefSB,0,1,0,1;\n" \
"                                    EV_500_Aggr1km_RefSB,1;\n" \
"                                    EV_250_Aggr1km_RefSB\n" \
"                               for the second and fourth bands of\n" \
"                               EV_1KM_RefSB, first band of\n" \
"                               EV_500_Aggr1km_RefSB, and all bands of\n" \
"                               EV_250_Aggr1km_RefSB \n" \
"                               Default is to process all swath SDSs. In\n" \
"                               this case, some SDSs may not be of the\n" \
"                               nominal MODIS scan size and therefore an\n" \
"                               error message will be output when the\n" \
"                               MRTSwath tries to process them.\n" \
"    -kk=kernel                 Resampling kernel type (CC, NN, or BI) \n" \
"                               Default is NN.\n" \
"    -oproj=output projection   Output projection number or short name. \n" \
"                               AEA, ER, GEO, GOODE, HAMMER, ISIN, LAMAZ, \n" \
"                               LAMCC, MERCAT, MOLL, PS, SNSOID, TM, and UTM\n"\
"    -oprm=output projection parameters  Output projection parameters (up\n" \
"                               to 15).  Defaults to all zeros.\n" \
"                               Lat/Long values are entered in decimal\n" \
"                               degrees\n" \
"    -opsz=output pixel sizes   Output pixel sizes (up to the number of \n" \
"                               SDSs being processed)  If the number of \n" \
"                               output pixel sizes is less than the \n"\
"                               number of SDSs, then the last pixel size \n" \
"                               is used for the rest of the pixel sizes. \n" \
"                               If the output pixel size is not \n" \
"                               specified, then the resolution of the \n" \
"                               input product will be determined and \n" \
"                               used as the output pixel size. \n" \
"                               Units is meters for all projections except\n" \
"                               GEO, which is degrees.\n" \
"    -oul=output upper left corner  Upper left corner in output space\n" \
"                               (long, lat in decimal degrees)\n" \
"                               (output proj x,y in meters: degrees for Geo)\n"\
"                               (input sample, line - in the case of \n" \
"                               multires products, specify the line/sample \n"\
"                               for the highest resolution SDS to be\n" \
"                               processed) Default is to use the bounding\n" \
"                               coords.\n" \
"    -olr=output lower right corner  Lower right corner in output space\n" \
"                               (long, lat in decimal degrees)\n" \
"                               (output proj x,y in meters: degrees for Geo)\n"\
"                               (input sample, line - in the case of \n" \
"                               multires products, specify the line/sample \n"\
"                               for the highest resolution SDS to be\n" \
"                               processed) Default is to use the bounding\n" \
"                               coords.\n" \
"    -osst=output spatial subset type  Type of output spatial subset info\n" \
"                               (LAT_LONG, PROJ_COORDS, LINE_SAMPLE)\n" \
"                               Default is LAT_LONG.\n" \
"    -ozn=output zone number    Output zone number.  Only needed for UTM.\n" \
"    -osp=output sphere number  Output sphere number.  Not needed if \n" \
"                               first output projection parameter is set.\n" \
"    -oty=output data type      Output data type (CHAR8, UINT8, INT8, \n" \
"                               INT16, UINT16, INT32, UINT32)\n" \
"                               Default is same as input data type.\n" \
"    -pf=parameter file         Parameter file\n" \
"\n" \
"Examples:\n" \
"\n" \
"swath2grid -if=/Modis/testdata/MOD021KM.A2003283.1655.004.2003283232504.hdf\n"\
"  -gf=/Modis/testdata/MOD03.A2003283.1655.004.2003283225954.hdf\n" \
"  -of=mod021km_cc -off=HDF_FMT\n" \
"  -sds=\"EV_1KM_RefSB,0,1,0,0,1; EV_500_Aggr1km_RefSB,1,0; EV_250_Aggr1km_RefSB; EV_Band26\"\n" \
"  -kk=CC -oproj=UTM -oprm=0.0,0.0,0.0,0.0,0.0 -osp=8 -ozn=13 -opsz=1000.0\n" \
"  -oul=-104.481078,55.929465 -olr=-68.407364,34.232958\n\n" \
"swath2grid -if=/Modis/testdata/MOD021KM.A2003283.1655.004.2003283232504.hdf\n"\
"  -gf=/Modis/testdata/MOD03.A2003283.1655.004.2003283225954.hdf \n" \
"  -of=mod021km_cc -off=HDF_FMT \n" \
"  -sds=\"EV_1KM_RefSB,0,1,0,0,1; EV_500_Aggr1km_RefSB,1,0; EV_250_Aggr1km_RefSB; EV_Band26\"\n" \
"  -kk=CC -oproj=UTM -oprm=0.0,0.0,0.0,0.0,0.0 -osp=8 -ozn=13 -opsz=1000.0,500.0 \n" \
"  -oul=-104.481078,55.929465 -olr=-68.407364,34.232958\n\n" \
"swath2grid -if=/Modis/testdata/MOD021KM.A2003283.1655.004.2003283232504.hdf\n"\
"  -gf=/Modis/testdata/MOD03.A2003283.1655.004.2003283225954.hdf\n" \
"  -of=mod021km_nn -off=GEOTIFF_FMT\n" \
"  -kk=NN -oproj=GEO -oprm=0.0,0.0,0.0,0.0,0.0 -osp=8 -opsz=0.01,0.015\n" \
"  -oul=-104.481078,55.929465 -olr=-68.407364,34.232958\n\n" \
"swath2grid -if=/Modis/testdata/MOD021KM.A2003283.1655.004.2003283232504.hdf\n"\
"  -gf=/Modis/testdata/MOD03.A2003283.1655.004.2003283225954.hdf\n" \
"  -of=mod021km -oproj=UTM -oprm=0.0,0.0,0.0,0.0,0.0 -osp=8 -ozn=13\n" \
" \n"

#define USAGE \
"usage: \n" \
"     swath2grid -if=<input file> -of=<output file> -gf=<geolocation file> \n" \
"            [-off=<output file format (HDF_FMT, GEOTIFF_FMT, RB_FMT)>] \n" \
"            [-sds=<SDS name>] [-kk=<resampling type (NN, BI, CC)>] \n" \
"            -oproj=<output projection> \n" \
"            [-oprm=<output projection parameters>] \n" \
"            [-opsz=<output pixel sizes>] [-oul=<output upper left corner>]\n" \
"            [-olr=<output lower right corner>] [-ozn=<output zone number>]\n" \
"            [-osst=<output spatial subset type \n" \
"                    (LAT_LONG, PROJ_COORDS, LINE_SAMPLE)>]\n" \
"            [-osp=<output sphere number>] \n" \
"            [-oty=<output data type>] \n" \
"            [-pf=<parameter file>] \n" \
" \n" \
" For more information use \n" \
"     swath2grid -help \n" \
" For general help on map projections and spheres use\n" \
"     swath2grid -help=proj \n" \
" For help on using the parameter file (-pf option) use\n" \
"     swath2grid -help=param \n" \
" For help on a specific map projection use\n" \
"     swath2grid -help=<projection> \n" \
" \n"

#define GENERAL_PROJ_HEADER \
" \n" \
" 'swath2grid' accepts a number of projections and spheriods: \n" \
" \n"

#define GENERAL_PARAM \
" \n" \
" 'swath2grid' accepts options from a text parameter file when specified \n" \
"          using the -pf option.\n" \
" \n" \
"  The following options names can be specified in the parameter file: \n" \
" \n" \
"    INPUT_FILENAME  = <input file name>\n" \
"          Abbreviation: IF\n" \
"\n" \
"    OUTPUT_FILENAME                 = <output file name>\n" \
"          Abbrevation: OF\n" \
"          No file extension is needed, since the output file type is\n" \
"          specified in the OUTPUT_FILE_FORMAT.  If an extension of .hdf,\n" \
"          .hdr, or .tif is specified, then it will be ignored.\n" \
"\n" \
"    OUTPUT_FILE_FORMAT              = <output file format>\n" \
"          Values : HDF_FMT, GEOTIFF_FMT, RB_FMT\n" \
"          Default: HDF_FMT.\n" \
"\n" \
"    GEOLOCATION_FILENAME            = <name of geolocation file>\n" \
"          Abbreviation: GF\n" \
"\n" \
"    INPUT_SDS_NAME                  = <input SDS name>\n" \
"          Abbreviation: SDS\n" \
"          Default is to process all swath SDSs. In this case, some SDS may\n" \
"          not be of the nominal MODIS scan size and therefore an error\n" \
"          message will be output when the MRTSwath tries to process them.\n" \
"          e.g. -sds=Cloud_Mask,1 for the first band of SDS Cloud_Mask in\n" \
"                    MOD35 granule.\n" \
"          e.g. EV_1KM_RefSB,0,1,0,1; EV_500_Aggr1km_RefSB,1;\n" \
"               EV_250_Aggr1km_RefSB for the second and fourth bands of\n" \
"               EV_1KM_RefSB, first band of EV_500_Aggr1km_RefSB, and all\n" \
"               bands of EV_250_Aggr1km_RefSB\n" \
"\n" \
"    KERNEL_TYPE                     = <resampling kernel type>i\n" \
"         Abbreviation: KK\n" \
"         Values : NN, BI, or CC\n" \
"         Default: NN.\n" \
"\n" \
"    OUTPUT_PROJECTION_NUMBER        = <output projection short name>\n" \
"        Abbreviation: OPROJ\n" \
"        Values: AEA, ER, GEO, GOODE, HAMMER, ISIN, LAMAZ, LAMCC,\n" \
"                MERCAT, MOLL, PS, SNSOID, TM, and UTM\n" \
"\n" \
"    OUTPUT_PROJECTION_PARAMETER     = <output projection parameters>\n" \
"        Abbreviation: OPRM\n" \
"        lat/long values entered in decimal degrees\n" \
"\n" \
"    OUTPUT_PIXEL_SIZE               = <output pixel sizes>\n" \
"        Abbreviation: OPSZ\n" \
"        If the number of output pixel sizes is less than the number of\n" \
"        SDSs, then the last pixel size is used for the rest of the\n" \
"        pixel sizes.  Default is to determine the input resolution and\n" \
"        use the input resolution for the output pixel size.  Units is\n" \
"        meters for all projections except GEO, which is degrees.\n" \
"\n" \
"    OUTPUT_SPACE_UPPER_LEFT_CORNER  = <output upper left corner>\n" \
"        Abbreviation: OUL\n" \
"        Default is to use the bounding rectangular coords.\n" \
"        Values:\n" \
"            LAT_LONG:    LONG LAT in decimal degrees\n" \
"            PROJ_COORDS: Output PROJ X Y in meters (degrees for Geographic)\n"\
"            LINE_SAMPLE: Input sample/line - In the case of multires \n" \
"                         products, specify the line/sample for the \n" \
"                         highest resolution SDS to be processed.\n" \
"\n" \
"    OUTPUT_SPACE_LOWER_RIGHT_CORNER = <output lower right corner> \n" \
"        Abbreviation: OLR\n" \
"        Default is to use the bounding rectangular coords.\n" \
"        Values:\n" \
"            LAT_LONG:    LONG LAT in decimal degrees\n" \
"            PROJ_COORDS: Output PROJ X Y in meters (degrees for Geographic)\n"\
"            LINE_SAMPLE: Input sample/line - In the case of multires \n" \
"                         products, specify the line/sample for the \n" \
"                         highest resolution SDS to be processed.\n" \
"\n" \
"    OUTPUT_SPATIAL_SUBSET_TYPE      = <output spatial subset type>\n" \
"        Abbreviation: OSST\n" \
"        Values : LAT_LONG, PROJ_COORDS, LINE_SAMPLE\n" \
"        Default: LAT_LONG.\n" \
"\n" \
"    OUTPUT_PROJECTION_SPHERE        = <output projection sphere>\n" \
"        Abbreviation: OSP) \n" \
"        Not needed if the first one/two projection parameters are\n" \
"        specified.\n" \
"\n" \
"    OUTPUT_PROJECTION_ZONE          = <output projection zone>\n" \
"        Abbreviation: OZN\n" \
"        Only used for UTM projection.\n" \
"\n" \
"    OUTPUT_DATA_TYPE                = <output data type>\n" \
"        Abbreviation: OTY\n" \
"        Default is to use the input data type.\n" \
"        Values: CHAR8, UINT8, INT8, INT16, UINT16, INT32, UINT32\n" \
" \n" \
"  Sample parameter files are available in the bin directory.\n" \
" \n" \
"  Values for options specified in the parameter file are overwritten by\n" \
"  command line options appearing after -pf.\n" \
" \n"

#define GENERAL_PROJ_TRAILER \
" \n" \
" Spheriods: \n" \
" \n" \
" For output spheres, either the spheriod number parameter (-osp)\n" \
" or the first two projection parameters (-oprm) can be used.  If the\n" \
" spheriod number is used, the first two projection parameters are ignored.\n" \
" For an ellipsoid, use the first projection parameter for the semi-major\n" \
" axis and the second for semi-minor axis of the ellipsoid.  For a sphere,\n" \
" use the first parameter for the radius and set the second parameter to\n" \
" zero.\n" \
" \n" \
" Projections:\n" \
"\n" \
" For output projections, enter both the projection (-oproj) and the\n" \
" projection parameters (-oprm).  Either the projection number or\n" \
" projection short name can be entered. If not all of the projection\n" \
" parameters are entered, they default to zero. The (null value)\n" \
" parameters should be set to zero. The output projection information\n" \
" is always required.\n" \
"\n" \
" Packed angle formats:\n" \
" \n" \
" Output Projection Parameters: Latitudes, longtitudes and angles are\n" \
" entered using decimal degrees.\n" \
" For instance, West 15 degrees, 30 minutes, 0 seconds, is entered\n" \
" as -15.50\n" \
"\n" \
" Zone number:\n" \
"\n" \
" The zone number parameter (-ozn) is only used for UTM.\n" \
"\n" \
" For help on specific map projection parameters use \n" \
"     swath2grid -help=<projection>\n" \
"\n"

#endif
