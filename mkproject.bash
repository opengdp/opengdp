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


##### name of project #####

echo
unset fullproject

while ! [ -n "$fullproject" ]
do
    read -e -i "Deep Water Horizon" -p "enter the full project name: " fullproject
done


##### name of project #####

echo
unset project

while ! [ -n "$project" ]
do
    read -e -i "deephorizon" -p "enter the project name: " project
done

project=$(echo "$project" | sed -r 's/[ \t]+/_/g')

    
##### project creation path #####

while true
do
    echo
    unset path
    while ! [ -n "$path" ]
    do
        read -e -i "/storage/data/" -p "enter the path to create the project in : " path
    done

    if ! [ -d "$path" ]
    then
        echo "ERROR: $path is not a dir"
        continue;
    fi
    
    if [ -e "${path}/${project}" ] && ! [ -d "${path}/${project}/" ]
    then
        echo "ERROR: $path exists and is not a dir"
        continue;
    fi
    
    if [ -d "${path}/${project}/" ]
    then
        unset t
        read -e -p "WARNING: ${path}/${project} already exists use anyway? yes/no : " t
        if [[ "$t" = "yes" ]]
        then
            break;
        fi
        continue;
    fi
    
    if ! [ -w "$path" ]
    then
        unset t
        read -e -p "WARNING ${path} is not writable use sudo? yes/no : " t
        if [[ "$t" = "yes" ]]
        then
            uid=$(id -u)
            gid=$(id -g)
            if sudo bash -c "mkdir \"${path}/${project}\" && chown ${uid}:${gid} \"${path}/${project}\""
            then
                break;
            fi
        fi
        continue;
    fi
    
    if mkdir "${path}/${project}"
    then
        break
    fi
    
done

##### edcftp base url #####

unset edcftp
echo

while ! [ -n "$edcftp" ]
do
    read -e -i "http://edcftp.cr.usgs.gov/pub/data/disaster/201004_Oilspill_GulfOfMexico/data/" -p "enter the edcftp base url: " edcftp
done

##### mapfile #####

unset mapfile
echo

while ! [ -n "$mapfile" ]
do
    read -e -i "$project.map" -p "enter the name of the mapfile: " mapfile
done

##### temp dir #####

while true
do
    echo
    unset tmp
    while ! [ -n "$tmp" ]
    do
        read -e -i "/mnt/ram2/" -p "enter the path to the temp dir: " tmp
    done
    
    if ! [ -d "$tmp" ]
    then
        echo "ERROR: $tmp is not a dir"
        continue;
    fi
    
    if ! [ -w "$tmp" ]
    then
        echo "ERROR: $tmp is not writeable"
        continue;
    fi

    break;
done

##### mapserver path #####

unset mapserverpath
echo

while ! [ -n "$mapserverpath" ]
do
    read -e -i "/usr/local/src/mapserver/mapserver/" -p "enter the path to the mapserver binaries: " mapserverpath
done

echo

##### limit #####

unset limit
echo

regex="^[0-9]+$"
while ! [[ $limit =~ $regex ]]
do
    read -e -i "12" -p "enter the max number of proc to spawn: " limit
done

##### cgibin  dir #####

while true
do
    echo
    unset cgibindir
    while ! [ -n "$cgibindir" ]
    do
        read -e -i "/var/www/cgi-bin" -p "enter the path to the cgi-bin dir: " cgibindir
    done
    
        if ! [ -d "$cgibindir" ]
    then
        echo "ERROR: $cgibindir is not a dir"
        continue;
    fi
    
    break
done

##### cgi bin script #####

while true
do
    echo
    unset cgibin
    while ! [ -n "$cgibin" ]
    do
        read -e -i "${project}" -p "enter the name of the cgi-bin script: " cgibin
    done
    
    if [ -e "$cgibindir/${cgibin}" ]
    then
        unset t
        read -e -p "WARNING: "$cgibindir/${cgibin}" already exists use anyway? yes/no : " t
        if [[ "$t" = "yes" ]]
        then
            break;
        fi
        continue;
    fi
     
    break;
done

##### html  dir #####

while true
do
    echo
    unset htmldir
    while ! [ -n "$htmldir" ]
    do
        read -e -i "/var/www/html/" -p "enter the path to the html dir to create the project in: " htmldir
    done
    

    if ! [ -d "$htmldir" ]
    then
        echo "ERROR: $htmldir is not a dir"
        continue;
    fi
    
    if [ -e "$htmldir/${project}" ] && ! [ -d "$htmldir/${project}/" ]
    then
        echo "ERROR: $htmldir exists and is not a dir"
        continue;
    fi
    
    if [ -d "$htmldir/${project}/" ]
    then
        unset t
        read -e -p "WARNING: $htmldir/${project} already exists use anyway? yes/no : " t
        if [[ "$t" = "yes" ]]
        then
            break;
        fi
        continue;
    fi
    
    if ! [ -w "$htmldir" ]
    then
        unset t
        read -e -p "WARNING $htmldir is not writable use sudo? yes/no : " t
        if [[ "$t" = "yes" ]]
        then
            uid=$(id -u)
            gid=$(id -g)
            if sudo bash -c "mkdir \"$htmldir/${project}\" && chown ${uid}:${gid} \"$htmldir/${project}\""
            then
                break;
            fi
        fi
        continue;
    fi
    
    if mkdir "$htmldir/${project}"
    then
        break
    fi
    
done


##### url base #####

unset urlbase
echo

while ! [ -n "$urlbase" ]
do
    read -e -i "http://$(hostname)" -p "enter the base url for the web server: " urlbase
done

echo

##### url cgi-bin dir #####

unset urlcgibindir
echo

while ! [ -n "$urlcgibindir" ]
do
    read -e -i "$urlbase/cgi-bin" -p "enter the cgi bin dir url: " urlcgibindir
done

##### url html dir #####

unset urlhtmldir
echo

while ! [ -n "$urlhtmldir" ]
do
    read -e -i "$urlbase/html/$project" -p "enter the html dir url: " urlhtmldir
done

##### some compound vars #####

basedir="${path}/${project}"
scriptdir="${path}/${project}/scripts/"
mapfile="$basedir/$mapfile"
urlcgibin="$urlcgibindir/$cgibin"
htmlbase="$htmldir/$project"

##### print the stuff #####

echo
echo
echo "------------------------------------------------------------------------"
echo "fullproject:      $fullproject"
echo "project:          $project"
echo "path:             $path"
echo "basedir:          $basedir"
echo "scriptdir:        $scriptdir"
echo "edcftp:           $edcftp"
echo "mapfile:          $mapfile"
echo "mapserverpath:    $mapserverpath"
echo "tmp:              $tmp"
echo "limit:            $limit"
echo "cgibindir:        $cgibindir"
echo "cgibin:           $cgibin"
echo "htmldir:          $htmldir"
echo "htmlbase:         $htmlbase"
echo "urlbase:          $urlbase"
echo "urlcgibindir:     $urlcgibindir"
echo "urlcgibin:        $urlcgibin"
echo "urlhtmldir:       $urlhtmldir"
echo "------------------------------------------------------------------------"


					 



###############################################################################
# function to sub the vars in a file
###############################################################################

function do_subst {
    sed -e "s,[@]fullproject[@],$fullproject,g" \
        -e "s,[@]project[@],$project,g" \
        -e "s,[@]path[@],$path,g" \
        -e "s,[@]basedir[@],$basedir,g" \
        -e "s,[@]scriptdir[@],$scriptdir,g" \
        -e "s,[@]edcftp[@],$edcftp,g" \
        -e "s,[@]mapfile[@],$mapfile,g" \
        -e "s,[@]mapserverpath[@],$mapserverpath,g" \
        -e "s,[@]tmp[@],$tmp,g" \
        -e "s,[@]limit[@],$limit,g" \
        -e "s,[@]cgibindir[@],$cgibindir,g" \
        -e "s,[@]cgibin[@],$cgibin,g" \
        -e "s,[@]htmldir[@],$htmldir,g" \
        -e "s,[@]htmlbase[@],$htmlbase,g" \
        -e "s,[@]urlbase[@],$urlbase,g" \
        -e "s,[@]urlcgibindir[@],$urlcgibindir,g" \
        -e "s,[@]urlcgibin[@],$urlcgibin,g"
        -e "s,[@]urlhtmldir[@],$urlhtmldir,g"
       
}
exit

##### scripts #####

scripts="
AERIAL_NOAA.bash
AERIAL_NOAA_MOSAIC.bash
AERIAL_NASA.bash
AERIAL_NASA_AVIRIS.bash
AERIAL_NASA_UAVSAR.bash
AERIAL_USACE.bash
LANDSAT_TM_USGS.bash
LANDSAT_ETM+_USGS.bash
E01.bash
ASTER.bash
dwh-generic.bash"

if ! -d "$scriptdir" ]
then
    mkdir "$scriptdir"
fi

for s in $scripts
do
    do_subst < "scripts/$s" > "$scriptdir/$s"
    chmod +x "$scriptdir/$s"
done



##### map #####

do_subst < "map/dwh.map" > "$mapfile"

##### cgi #####

echo "Installing cgi-bin"

sudo bash -c "do_subst < \"cgi-bin/dwh\" > \"$cgibindir/${cgibin}\" && chown +x \"$cgibindir/${cgibin}\""

##### html and js #####

do_subst < "html/index.html" > "$htmlbase/index.html"
do_subst < "html/setup.js" > "$htmlbase/setup.js"
do_subst < "html/google.js" > "$htmlbase/google.js"
do_subst < "html/finish.js" > "$htmlbase/finish.js"

