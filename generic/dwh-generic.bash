#!/bin/bash
# Copyright (c) 2010, Brian Case
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.




source dwh-misc-generic.bash
source dwh-addo-generic.bash
source dwh-image-generic.bash
source dwh-file-generic.bash
source dwh-mapfile-generic.bash
source dwh-ms_overview-generic.bash
source dwh-geoext-generic.bash
source dwh-tc-generic.bash
source dwh-pan-generic.bash

source dwh-proj_def.bash




###############################################################################
# est completion time meter
###############################################################################

function comp_meter {

    started=$1
    lines=$2
    donelines=$3
    
    decdone=$(bc <<< "scale = 6; $donelines / $lines")
    percdone=$(bc <<< "scale = 0; $decdone * 100")
    elap=$(($(date +%s) - started))
    comp=$(bc <<< "scale=0; $elap / $decdone")
    ((comp +=  started))
    
    printf "\r%3.0f%% complete. EST. finish at %s" $percdone "$(date -d "@${comp}")"
}

        	

###############################################################################
# multi proceessing loop
###############################################################################

function mainloop {
    local mirrorfile="$1"
    local dofunc="$2"
    
    ((doing=0))
    
    ##### open a fd to a named pipe #####

    mkfifo pipe; exec 3<>pipe
    
    ##### setup for the est completion time #####
    
    lines=$(grep "${mirrorfile}" -e "^get" | wc -l  | cut -d " " -f 1 )
    ((donelines=0))
    started=$(date +%s)
    
    ##### loop over the list #####

    while read line ;
    do
        
        ##### if it is a mkdir command do it now #####
        
        if grep -e "^mkdir" <<< "$line" > /dev/null
        then
            lftp -e "$line ; exit"
            continue
        fi
        
        ##### under the limit just start a job #####

        if [ $doing -lt $limit ]
        then
        	${dofunc} "$line"  &
         	((doing++))
         	
        ##### over the limit wait for a job to finish before starting #####
        
        else
            read <&3
            ((doing--))
            
            if grep -e "^get" <<< "$line" > /dev/null
            then
                ((donelines++))
            fi

            comp_meter $started $lines $donelines
            
            ${dofunc} "$line"  &
         	((doing++))
        fi

    done < "${mirrorfile}"

    wait

    echo

}

###############################################################################
# usage
###############################################################################

function usage {

echo "$1 [ -h ] || [ -v ] || | [ -rebuild ] || [ -readdo ] || [ -retindex ] ||"
echo "                            [ -reover ] || [ -remapfile || [ -regeoext ]"
}

###############################################################################
# sub main function
###############################################################################

function sub_main {

    ((stage = 0))
    ((stop = 0))

    while [[ $1 == -* ]]; do
        case "$1" in
            -h|--help|-\?)
                usage $0
                exit 0
                ;;
            -v|--verbose)
                ((verbose=1))
                shift
                ;;
            -rebuild|--rebuild)
                ((stage=1))
                shift
                ;;
            -readdo|--readdo)
                ((stage=2))
                shift
                ;;
            -retindex|--retindex)
                ((stage=3))
                shift
                ;;
            -reover|--reover)
                ((stage=4))
                shift
                ;;
            -remapfile|--remapfile)
                ((stage=5))
                shift
                ;;
            -regeoext|--regeoext)
                ((stage=6))
                shift
                ;;
            -stop|--stop)
                ((stop=1))
                shift
                ;;
            -findtile|--findtile)
                findtile="$2 $3"
                shift
                shift
                shift
                ;;
            -*)
                echo "invalid option: $1" 1>&2
                usage $0
                exit 1
                ;;
        esac
    done

    if ((stage == 0)) && ((stop == 1))
    then
        echo "Ignoreing -stop switch, no rebuild stage specified"
        ((stop = 0))
    fi

    ##### findtile #####
    
    if [ -n "$findtile" ]
    then
        
        ogrinfo "$outdir" -al -spat $findtile $findtile | grep "  location (String) = "
        exit
    fi
    

    ##### check if called for a rebuild #####
    
    if (( stage == 1 ))
    then
        echo "rebuilding"
        DWH_REBUILD="rebuild"
        if ! mv "$indir" "${indir/%\//}.old"
        then
            exit
        fi

        if ! mv "$outdir" "${outdir/%\//}.old"
        then
            exit
        fi

        if ! mkdir -p "$indir"
        then
            exit
        fi
        
        cd "$indir"

        baseurl="file://${indir/%\//}.old"

    fi

    case "$stage" in
        
        ##### normal op or -rebuild #####
        0|1)   

            ##### get the list of new files to fetch #####
                
            if ! getlist "$mirrorfile" "$fetchpattern"
            then
                exit
            fi
        
            ##### loop over the commands in the mirrorfile #####
        
            if ! [ -n "$dofunc" ] ; then dofunc="dofile" ; fi
        
            if ! mainloop "$mirrorfile" "$dofunc"
            then
                exit
            fi
    
            if ((stop == 1))
            then
                ##### clean up after a -rebuild #####

                if (( stage == 1 ))
                then
                    echo "rebuilding"
                    rm -r "$indir"

                    mv "${indir/%\//}.old" "$indir"
                
                fi
                exit
            fi
        ;;
    esac
    case "$stage" in
        ##### -readdo only #####
        2)
            readdo
            
            if ((stop == 1))
            then
                exit
            fi
        ;;
    esac
    case "$stage" in
        ##### -retindex only #####
        3)
            rebuildtindexs
            
            if ((stop == 1))
            then
                exit
            fi
        ;;
    esac
    case "$stage" in
        ##### -reover or before #####
        0|1|2|3|4)
            if (( stage == 0 ))
            then
                grep "${mirrorfile}" -e "^get" |\
                ${datefunc}
            else
                find "${outdir}" -type d -name "[0-9]*" |\
                sed 's:.*/::g'
             fi |\
             sort |\
             uniq |\
             while read ts
            do
                
                ##### add an include line in the main mapfile #####

                addinclude "$ts"

                ##### add an include line in the main Newworld mapfile #####

                addinclude_NewWorld "$ts"

        
                ##### get the extent of the ds #####
       
                extent=$(getextent "$ts")
                
                if [[ "$doovr" == "yes" ]]
                then

                    ##### write a temp map file #####

                    writemap_noover "$ts" "$extent"

                    ##### create a single file for larger area overviews #####
                
                    scale=$(makeoverview "$ts" "$extent")
                    
                    ##### write the map file with the correct scale #####

                    writemap_withover "$ts" "$extent" "$scale"

                else
                    
                    ##### write the map file with no min max layers #####
                    
                    writemap_noover "$ts" "$extent"
                    
                fi
                
                ##### create a map file for new world #####

                writemap_NewWorld "$ts" "$extent"

            done
        

            if ((stop == 1))
            then
                exit
            fi
        ;;
    esac
    case "$stage" in
        ##### -remapfile only #####
        5)
  
            find "${outdir}" -type d -name "[0-9]*" |\
             sed 's:.*/::g' |\
             sort |\
             uniq |\
             while read ts
            do  
                
                ##### add an include line in the main mapfile #####

                addinclude "$ts"
                
                ##### add an include line in the main Newworld mapfile #####

                if [ -n "$NewWorld_mapfile" ]
                then
                    addinclude_NewWorld "$ts"
                fi
                
                ##### get the extent of the ds #####

                extent=$(getextent "$ts")

                if [[ "$doovr" == "yes" ]]
                then

                    ##### get the overview scale from the old mapfile #####

                    scale=$(getoverviewscale "$ts")
                    
                    ##### write the map file with the correct scale #####

                    writemap_withover "$ts" "$extent" "$scale"

                else
                    
                    ##### write the map file with no min max layers #####
                    
                    writemap_noover "$ts" "$extent"
                    
                fi
                
                ##### create a map file for new world #####

                if [ -n "$NewWorld_mapfile" ]
                then
                    writemap_NewWorld "$ts" "$extent"
                fi

            done
        
            if ((stop == 1))
            then
                exit
            fi
        ;;
    esac
    case "$stage" in
        0|1|2|3|4|5|6)

            ##### stage 6 and before #####
            #####  write out a js file for geoext #####
                
            dogeoext
            dogeoext_tc

            ##### write out a config section fore tilecache #####

            dotc
            if ((stop == 1))
            then
                exit
            fi
        ;;
    esac

    ##### clean up after a -rebuild #####

    if (( stage == 1 ))
    then
        echo "rebuilding"
        rm -r "$indir"
        mv "${indir/%\//}.old" "$indir"
            
    fi

}
###############################################################################
# main
###############################################################################

function main {
    
    ##### project #####

    if ! [[ -n "${project}" ]]
    then
        echo "ERROR: var project not set"
        exit
    fi
    

    ##### dsname #####
    
    if ! [[ -n "${dsname}" ]]
    then
        echo "ERROR: var dsname not set"
        exit
    fi
    
    ##### fetch url #####
    
    if ! [[ -n "${baseurl}" ]]
    then
        echo "ERROR: var baseurl not set"
        exit
    fi
    
    ##### basedir #####
    
    if ! [[ -n "${basedir}" ]]
    then
        echo "ERROR: var basedir not set"
        exit
    fi
    
    if ! [ -d "$basedir" ]
    then
        echo "ERROR: no such dir $basedir"
        exit
    fi
    
    if ! [ -w "$basedir" ]
    then
        echo "ERROR: no write access to $basedir"
        exit
    fi
    
    ##### indir #####
    
    if ! [[ -n "${indir}" ]]
    then
        echo "ERROR: var indir not set"
        exit
    fi
    
    
    if ! [ -d "$indir" ]
    then
        if ! mkdir -p "$indir"
        then
            exit
        fi
    fi
    
    if ! [ -w "$indir" ]
    then
        echo "ERROR: no write access to $indir"
        exit
    fi
    
    ##### outdir #####
    
    if ! [[ -n "${outdir}" ]]
    then
        echo "ERROR: var outdir not set"
        exit
    fi
    
    if ! [ -d "$outdir" ]
    then
        if ! mkdir -p "$outdir"
        then
            exit
        fi
    fi
    
    if ! [ -w "$outdir" ]
    then
        echo "ERROR: no write access to $outdir"
        exit
    fi
    
    ##### tmp dir #####
    
    if ! [ -n "$tmp" ] ; then tmp="/tmp/" ; fi
    
    if ! [ -d "$tmp" ]
    then
        echo "ERROR: no such dir $tmp"
        exit
    fi
    
    if ! [ -w "$tmp" ]
    then
        echo "ERROR: no write access to $tmp"
        exit
    fi
    
    ##### mapfile #####
    
    if ! [[ -n "${mapfile}" ]]
    then
        echo "ERROR: var mapfile not set"
        exit
    fi
    
    if ! [ -f "$mapfile" ]
    then
        write_main_map
    fi

    if ! [ -f "$mapfile" ]
    then
        echo "ERROR: no such file $mapfile"
        exit
    fi

    if ! [ -w "$mapfile" ]
    then
        echo "ERROR: no write access to $mapfile"
        exit
    fi
    
    ##### mapserverpath #####
    
    if ! [ -n "$mapserverpath" ] ; then mapserverpath="/usr/local/src/mapserver/mapserver/" ; fi
    
    if ! [ -d "$mapserverpath" ]
    then
        echo "ERROR: no such dir $mapserverpath"
        exit
    fi
    
    if ! [ -x "${mapserverpath}/shp2img" ]
    then
        echo "ERROR: no executable ${mapserverpath}/shp2img"
        exit
    fi
    
    ##### do ovrview default #####
    
    if ! [ -n "$doovr" ] ; then doovr="yes" ; fi

    ##### fetch pattern default #####
    
    if ! [ -n "$fetchpattern" ] ; then fetchpattern="*" ; fi
    
    ##### unzip untar... pattern default #####
    
    if ! [ -n "$extglob" ] ; then extglob="*.tif" ; fi
    
    ##### unzip kmz pattern default #####
    
    if ! [ -n "$baseglob" ] ; then baseglob="tile-*" ; fi
    
    ##### proccess limit default #####
    
    if ! [ -n "$limit" ] ; then limit="4" ; fi
    
    if ! [ -n "$datefunc" ] ; then datefunc="dodate" ; fi

    ##### cd to the in dir #####

    cd "$indir"

    ##### file name for the mirror file #####
    
    host="$(hostname)"
    mirrorfile="$host.mirror.lftp"
    
    sub_main "$@"

}


