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

if ! which dialog
then
    echo "this script requires the dialog utility"
    exit 1
fi

function inputbox {
    echo $(dialog --clear --inputbox "$1" 12 80 "$2" 2>&1 >/dev/tty)
}

function yesnobox {
    
    dialog --clear --yesno "$1" 12 80  2>&1 >/dev/tty
}

##### name of project #####

unset fullproject

while ! [ -n "$fullproject" ]
do
    fullproject="$(inputbox "enter the full project name: " "Deep Water Horizon")"
done


##### name of project #####

unset project

while ! [ -n "$project" ]
do
    project="$(inputbox "enter the project name: " "deephorizon")"
done

project=$(echo "$project" | sed -r 's/[ \t]+/_/g')

    
##### project creation path #####

error=""

while true
do
    unset path
    while ! [ -n "$path" ]
    do
        path="$(inputbox "${error}enter the path to create the project in : " "/storage/data/")"
    done

    if ! [ -d "$path" ]
    then
        error="ERROR: $path is not a dir\n"
        continue;
    fi
    
    if [ -e "${path}/${project}" ] && ! [ -d "${path}/${project}/" ]
    then
        error="ERROR: $path exists and is not a dir\n"
        continue;
    fi
    
    if [ -d "${path}/${project}/" ]
    then
        if yesnobox "WARNING: ${path}/${project} already exists use anyway? yes/no : "
        then
            break;
        fi
        continue;
    fi
    
    if ! [ -w "$path" ]
    then
        if yesnobox "WARNING ${path} is not writable use sudo? yes/no : "
        then
            uid=$(id -u)
            gid=$(id -g)
            reset
            if sudo bash -c "$(printf 'mkdir %q' "${path}/${project}") && $(printf 'chown %q %q' ${uid}:${gid} "${path}/${project}") "
            then
                break;
            fi
            error="mkdir failed\n"
        fi
        continue;
    fi
    
    if mkdir "${path}/${project}"
    then
        break
    fi
    
    error="mkdir failed\n"
    
done

##### edcftp base url #####

unset edcftp

while ! [ -n "$edcftp" ]
do
    edcftp=$(inputbox "enter the edcftp base url: " "http://edcftp.cr.usgs.gov/pub/data/disaster/201004_Oilspill_GulfOfMexico/data/")
done

##### mapfile #####

unset mapfile

while ! [ -n "$mapfile" ]
do
    mapfile=$(inputbox "enter the name of the mapfile: " "$project.map")
done

##### temp dir #####

error=""

while true
do
    unset tmpdir
    while ! [ -n "$tmp" ]
    do
        tmpdir=$(inputbox "${error}enter the path to the temp dir: " "/mnt/ram2/")
    done
    
    if ! [ -d "$tmpdir" ]
    then
        error="ERROR: $tmpdir is not a dir\n"
        continue;
    fi
    
    if ! [ -w "$tmpdir" ]
    then
        error="ERROR: $tmpdir is not writeable\n"
        continue;
    fi

    break;
done

##### mapserver path #####

unset mapserverpath

while ! [ -n "$mapserverpath" ]
do
    mapserverpath=$(inputbox "enter the path to the mapserver binaries: " "/usr/local/src/mapserver/mapserver/")
done

##### limit #####

unset limit

regex="^[0-9]+$"
while ! [[ $limit =~ $regex ]]
do
    limit=$(inputbox "enter the max number of proc to spawn: " "12")
done

##### cgibin  dir #####

unset error
while true
do
    unset cgibindir
    while ! [ -n "$cgibindir" ]
    do
        cgibindir=$(inputbox "${error}enter the path to the cgi-bin dir: " "/var/www/cgi-bin")
    done
    
        if ! [ -d "$cgibindir" ]
    then
        error="ERROR: $cgibindir is not a dir\n"
        continue;
    fi
    
    break
done

##### cgi bin script #####

while true
do
    unset cgibin
    while ! [ -n "$cgibin" ]
    do
        cgibin=$(inputbox "enter the name of the cgi-bin script: " "${project}")
    done
    
    if [ -e "$cgibindir/${cgibin}" ]
    then
        if yesnobox "WARNING: "$cgibindir/${cgibin}" already exists use anyway? yes/no : "
        then
            break;
        fi
        continue;
    fi
     
    break;
done

##### html  dir #####

unset error

while true
do
    unset htmldir
    while ! [ -n "$htmldir" ]
    do
        htmldir=$(inputbox "${error}enter the path to the html dir to create the project in: " "/var/www/html/")
    done
    

    if ! [ -d "$htmldir" ]
    then
        error="ERROR: $htmldir is not a dir\n"
        continue;
    fi
    
    if [ -e "$htmldir/${project}" ] && ! [ -d "$htmldir/${project}/" ]
    then
        error="ERROR: $htmldir exists and is not a dir\n"
        continue;
    fi
    
    if [ -d "$htmldir/${project}/" ]
    then
        if yesnobox "WARNING: $htmldir/${project} already exists use anyway? yes/no : "
        then
            break;
        fi
        continue;
    fi
    
    if ! [ -w "$htmldir" ]
    then
        if yesnobox "WARNING $htmldir is not writable use sudo? yes/no : "
        then
            uid=$(id -u)
            gid=$(id -g)
            reset
            if sudo bash -c "$(printf 'mkdir %q' "$htmldir/${project}") && $(printf 'chown %q %q' ${uid}:${gid} "$htmldir/${project}")"
            then
                break;
            fi
            error="mkdir failed\n"
        fi
        continue;
    fi
    
    if mkdir "$htmldir/${project}"
    then
        break
    fi
    
    error="mkdir failed\n"
done


##### url base #####

unset urlbase

while ! [ -n "$urlbase" ]
do
    urlbase=$(inputbox "enter the base url for the web server: " "http://$(hostname)")
done

##### url cgi-bin dir #####

unset urlcgibindir

while ! [ -n "$urlcgibindir" ]
do
    urlcgibindir=$(inputbox "enter the cgi bin dir url: " "$urlbase/cgi-bin")
done

##### url html dir #####

unset urlhtmldir


while ! [ -n "$urlhtmldir" ]
do
    urlhtmldir=$(inputbox "enter the html dir url" "$urlbase/html/$project")
done

##### some compound vars #####

basedir="${path}/${project}"
scriptdir="${path}/${project}/scripts/"
mapfile="$basedir/$mapfile"
urlcgibin="$urlcgibindir/$cgibin"
htmlbase="$htmldir/$project"

##### print the stuff #####




dialog --clear --msgbox "$(
echo "------------------------------------------------------------------------"
echo "fullproject:      $fullproject"
echo "project:          $project"
echo "path:             $path"
echo "basedir:          $basedir"
echo "scriptdir:        $scriptdir"
echo "edcftp:           $edcftp"
echo "mapfile:          $mapfile"
echo "mapserverpath:    $mapserverpath"
echo "tmpdir:           $tmpdir"
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
)" 30 100

					 

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
        -e "s,[@]tmp[@],$tmpdir,g" \
        -e "s,[@]limit[@],$limit,g" \
        -e "s,[@]cgibindir[@],$cgibindir,g" \
        -e "s,[@]cgibin[@],$cgibin,g" \
        -e "s,[@]htmldir[@],$htmldir,g" \
        -e "s,[@]htmlbase[@],$htmlbase,g" \
        -e "s,[@]urlbase[@],$urlbase,g" \
        -e "s,[@]urlcgibindir[@],$urlcgibindir,g" \
        -e "s,[@]urlcgibin[@],$urlcgibin,g" \
        -e "s,[@]urlhtmldir[@],$urlhtmldir,g"
       
}


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

if [ ! -d "$scriptdir" ]
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

tmp=$(mktemp -t "${dwh}XXXXXXXXXX")
do_subst < "cgi-bin/dwh" >> "$tmp"

sudo bash -c "$(printf 'mv %q %q' "$tmp" "$cgibindir/${cgibin}") && $(printf 'chmod +x %q' "$cgibindir/${cgibin}")"

rm -f "$tmp"


##### html and js #####

do_subst < "html/index.html" > "$htmlbase/index.html"
do_subst < "html/setup.js" > "$htmlbase/setup.js"
do_subst < "html/google.js" > "$htmlbase/google.js"
do_subst < "html/finish.js" > "$htmlbase/finish.js"

