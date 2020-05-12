#!/bin/sh

#Initialize variables

NONE=0

#Change type is assigned 1 or combination of 2...
#Constants are capital case

LOWERCASE=1
UPPERCASE=2
SED_MODE=3
RECURSIVE=10

#Variables are lower case separeted with underscore  

change_type=$NONE
sed_pattern=""
script_name=`basename $0`

#Usage message
#Prints usage rules till EOT

get_help() {       
cat<<EOT 
  Usage rules:
  The command requires minimum of 2 parameters to work
  $0 [-r] [-l|-u] <dir/file names...>
  $0 [-r] <sed pattern> <dir/file names...>
  $0 [-h]

  Where:
	-l lowerizes specified filenames
	-u uppersizes specified filenames
	-r runs the script with recursion
	-h prints this help
EOT
exit

}

#--------------------------------------------------------------------------------------------------------------------
#Modifies_a_file modifies the name of a single file

modify_a_file(){

                current_file_name=`basename "$file_name"` #Take the file name and path of the file
                current_path=`dirname "$file_name"`
                 
                if [ "$current_file_name" = "$script_name" ] ; #Don't modify the script
		then
                	return
                fi
 
                case $change_type in
                        1|11) new_name=`echo "$current_file_name" |  tr [:upper:] [:lower:]` ;; #Change to lowercase
                        2|12) new_name=`echo "$current_file_name" |  tr [:lower:] [:upper:]` ;; #Change to uppercase
                        3|13) new_name=`echo "$current_file_name" |  sed -s $sed_pattern` ;; #Apply sed_pattern
                esac

   		
                if [ -f "$current_path/$new_name" ] ; #Check if new file already exists then name modified is failed
		then
                	`echo "File $new_name already exists, name modifying is failed" 1>&2`
                        return

                else #Rename the file                         
   			mv "$current_path/$current_file_name" "$current_path/$new_name"
			echo "The $current_file_name is changed to $new_name"

                fi  
		
   
}
# --------------------------------------------------------------------------------------------------------------------------
#Chek the file name if it is a folder

check_if_folder(){	


        if [ ! -f "$file_name" -a ! -d "$file_name" ] ; #Check if the file exists 
	then
        	`echo "Neither file name nor directory --$1-- exists" 1>&2`
		get_help
                exit
        fi 	


        if [ ! -w "$file_name" -a -f "$file_name" ]; #Check if the file could be modified 
	then
        		`echo "File $file_name cannot be modified" 1>&2`
                	exit
        fi


	if [ $change_type -gt $RECURSIVE -a -d $file_name ] ; #If change_type is greater than recursive and a file name then do to recursive_change function 
	then
		recursive_change 
	
	elif [ $change_type -lt $RECURSIVE -a -f $file_name ] ; #Else if change_type is lower than recursive and a file name then do to modify_a_file function 
	then
		modify_a_file

	else #Else goes to get_help
		get_help
	fi

}

#--------------------------------------------------------------------------------------------------------------------
#Recursive_change function when there is a recursion

recursive_change () {

	recursive_dir_name=$file_name
	
	for recursive_file_name in $(find $recursive_dir_name -depth -type f) #Use find comment to recurse into directory tree and return all the file names in the tree
	do 	
  		file_name=$recursive_file_name
		modify_a_file
	done

}


#-------------------------------------------------------------------------------------------------------------------------
#Body of the script

if [ $# -eq 0 ]; #When there is not enough parameters, usage rules are needed.  
then
        get_help
fi
 
while [ "$1" != "" ]; #Parse parameters
do
        case $1 in
                -r)  change_type=`expr $change_type + $RECURSIVE`;;
                -l)  change_type=`expr $change_type + $LOWERCASE` ;;
                -u)  change_type=`expr $change_type + $UPPERCASE` ;;
                -h)  get_help
                     exit ;;

                *)
                
		if [ $change_type = $NONE ]; #Only sed pattern is supplied
		then
			echo $change_type $SED_MODE
                        change_type=`expr $change_type + $SED_MODE`
                        sed_pattern=$1
                else
			file_name=$1 #Takes file which is given as a parameter
                        check_if_folder
                fi
                	;;
        esac
shift #Shifts parameters into previous ones position
done
