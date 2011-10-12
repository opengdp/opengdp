1. README file for Terra MOD_PRDS1KM (MODIS Atmosphere L1B 1KM Destriping process)


2. POINTS OF CONTACT:

   Developers:
   ----------
   Nazmi Saleous (adapted code to 1KM input)
   MODIS Land Science Team           
   Raytheon Systems 
   NASA's GSFC Code 923 
   Greenbelt, MD 20771
   phone: 301-614-6647
   nazmi.saleous@gsfc.nasa.gov      

   Liam Gumley (developer of original MOD_PRDS)
   CIMSS/SSEC
   1225 W. Dayton St.
   Madison WI 53706
   (608) 265-5358
   Liam.Gumley@ssec.wisc.edu

   SDST Contacts:
   -------------
   Carol Davidson	301-352-2159	cdavidson@saicmodis.com
   Gwyn Fireman		301-352-2118    gfireman@saicmodis.com
   SAIC-GSO MODIS SCIENCE DATA SUPPORT OFFICE
   7501 Forbes Boulevard, Suite 103, Seabrook, MD 20706
   Fax: 301-352-0143


3. BUILD INSTRUCTIONS:
  a. To build the Destriping code, first be sure all environment variables
     required by the makefile are set. (The list of required environment 
     variables is provided in Item 4 below.)  At TLCF, execute the following 
     commands at the UNIX command line:

        setenv ROOT_DIR /cc/cc_vob
        source $ROOT_DIR/COMMON/MODAPS_setup_sdp529
        64_f90

     where MODAPS_setup_sdp529 is the MODIS setup script.

     The HDF5 static or dynamic shared libraries (used by SDPTK 5.2.7v3 and 
     later) are needed during the link step and in the runtime environment 
     for shared library even though MODIS currently produces only HDF4, not 
     HDF5, products.  

     In this version, MOD_PRDS1KM is linked with the HDF5 static library rather
     than the dynamic library.  This will result in a substantial growth of 
     the executable, but it ensures no conflicting usage of the HDF5 shared
     libraries when switching between toolkit environments (f90 or f77) among
     the processes of PGE03.

  b. Be sure that the atmosphere status message include (PGS_MODIS_39500.f)
     and message (PGS_39500) files exist in the SDPTK $PGSINC and $PGSMSG
     directories, respectively, and that they are up-to-date with respect to
     the message text file MODIS_39500.t in directory
     $MODIS_STORE/shared_src/atmos_src/src_L2". ($MODIS_STORE is the home
     directory of the MODIS atmosphere shared source code on the host system.)

     If the two files do not exist, build them using the SDPTK SMF compile
     facility.  At the UNIX command line type:

     $PGSBIN/smfcompile -f $MODIS_STORE/shared_src/atmos_src/src_L2/MODIS_39500.t \
     -f77 -i -r

     Options -i and -r redirect the include and message files to directories
     $PGSBIN and $PGSMSG, respectively.

  c. Type "clearmake -f MOD_PRDS1KM.mk" to build the profiles executable, 
     MOD_PRDS1KM.exe.

  d. Before running, set the SDPTK environment variable PGS_PC_INFO_FILE to the
     full path name of the PCF, MOD_PRDS1KM.pcf. Edit the PCF to reflect the names
     and directories of the locations of input and output files on host system.

     Be sure to set the Runtime Parameters on LUNs 10258 and 10259 to the
     Collection Start and Stop times of the current granule.

     Make sure the files defined in Item 11 are in the correct PCF LUN.

     NOTE: An up-to-date PGSTK leapsec.dat file is required for setting and
     writing ECS metadata.

  e. To run the code, type MOD_PRDS1KM.exe


4. ENVIRONMENT VARIABLES USED IN THE MAKEFILE
   See "Env Variables" in MOD_PRDS1KM.mk prolog for list of system and user
   environment variables.


5. MAPI:

   SCF:  Version 2.3.4
   TLCF:


6. HDF:

   SCF: Version 4.1r5
   TLCF: Version 5.2.9v1.00 (including HDF-EOS Version 2.9)


7. SDP TOOLKIT:

   SCF: Version 5.2.10v1.00 (Includes HDF-EOS Version 2.10v1.00)
   TLCF:


8. PLATFORM INFORMATION:

   SCF:
   Hardware: Dell Intel P4 800 MHz, 2 GB RAM
   Operating System: Red Hat Linux 8.0
   Fortran 90 Compiler: Portland Group pgf90 5.1-3

   TLCF:
   Hardware: Silicon Graphics Origin2000 (R10000/194MHz, 2048 MB RAM)
   Operating System: Irix 6.5.11f 
   Fortran 90 Compiler: MIPSpro Compilers: Version 7.3.1


9. ANCILLARY DATA:

   None.


10. MODIS PRODUCTS AS INPUTS:
    MODIS L1B 1KM HDF file.


11. OTHER: Identify other types of input (such as Look Up Tables).

    NAMES				Description			PCF LUN
    -----				-----------			-------
    MOD021KM_destripe_config.dat.v1	Terra destriping configuration	430100
    MYD021KM_destripe_config.dat.v1	Aqua destriping configuration	430100


12. PROBLEMS: Identify known software problems/defects (e.g., memory leaks).
    None.


13. DELIVERY SIZE: Total size of untarred delivery: ??? KB


14. OUTPUT FILE SIZE: Size of expected output files.
    Size of the product files (MOD021KM) is the same as the input file.


15. TESTS PERFORMED:
    NOTE: The test output dataset was generated in *unoptimized* mode,
          with the DEBUG flag set to 1 in the PCF file.


16. EXIT CODES:
    0 - Success
    1 - Fail


17. ERROR_LIST
    Code generates LogStatus Error messages containing "operator action".
