#!/bin/bash

function help {
	echo "COCADA Collection Of Algorithms and DAta Structures"
    echo "(c) 2016- Paulo Fonseca"
	echo ""
}

INSTALL_DIR=$HOME/.cocada

function write_env_setup {
	ENV_FILE=$INSTALL_DIR/env.sh
	echo "#!/bin/sh" > $ENV_FILE
	echo "# COCADA environment setup" >> $ENV_FILE
	echo "export COCADA_HOME=\"$INSTALL_DIR\"" >> $ENV_FILE
	echo "case \":\${PATH}:\" in" >> $ENV_FILE
    echo "*:\"\$COCADA_HOME/bin\":*)" >> $ENV_FILE
    echo "	;;" >> $ENV_FILE
    echo "*)" >> $ENV_FILE
    echo "	export PATH=\"\$COCADA_HOME/bin:\$PATH\"" >> $ENV_FILE
    echo "	;;" >> $ENV_FILE
	echo "esac" >> $ENV_FILE
}

function patch_profile {
	write_env_setup
	echo ""
	echo "COCADA needs the environment variable \$CODADA_HOME to be defined and added to the PATH"
	case $SHELL in
	*"bash" )
		echo "It seems your system uses the bash shell. Great!"
		echo "I can patch the configuration files "
		echo "(Don't worry, I'll backup the original files)"
		echo "Proceed? (Y)es | (N)o "
		read -s -N 1 ANS
		if [[ "$ANS" == "Y" || "$ANS" == "y" || "$ANS" == "" ]]
		then
			echo "Patching configuration files"
			if [[ -e $HOME/.bash_profile ]]
			then
				cp $HOME/.bash_profile $HOME/.bash_profile.cocada_bkp.`date +%Y%m%d%H%M`
				echo "" >> $HOME/.bash_profile
				echo "# COCADA  environment setup" >> $HOME/.bash_profile
				echo ". $INSTALL_DIR/env.sh" >> $HOME/.bash_profile
			fi
			echo "Done"
		else 
			echo "OK, but please make the appropriate changes to your configuration files"
		fi
		;;
	* ) 
		echo "Please make the appropriate changes to your configuration files"
		;;
	esac
}



function install {
    echo "Enter install destination, or just press Return for default '$INSTALL_DIR':"
    read ANS
    if [[ ! "$ANS" == "" ]]
    then
        INSTALL_DIR=$ANS 
    fi 
    if [[ -d $INSTALL_DIR ]]
    then
		echo ""
        echo "WARNING: Install directory $INSTALL_DIR already exists!"
        echo "Proceeding will overwrite its contents."
        echo "Continue anyway? (Type \"Yes\" to confirm)"
        read ANS
        if [[ "$ANS" == "Yes" || "$ANS" == "yes" ]]
        then
            echo "Overwriting $INSTALL_DIR"
        else
			echo ""
            echo "Aborting installation."
            exit 1
        fi
    fi
    mkdir -p $INSTALL_DIR
	cd $INSTALL_DIR
	patch_profile 

}

if [ $# -lt 1 ]; 
then
	help
	exit 0
fi

OPERATION=
TARGETS=
OPTION=
NOWWHAT="op"

while [ "$1" != "" ]; do
	case $1 in
		"help" ) if [ ! "$NOWWHAT" == "op" ];
			then
				help
				exit 1
			fi
			help
			exit 0
			;;
		"install" ) if [ ! "$NOWWHAT" == "op" ];
			then
				help
				exit 1
			fi
			OPERATION="install"
            shift
			;;
		* ) ;;
	esac
done

echo $OPERATION $TARGETS $OPTION

case $OPERATION in
	"install" )	if [ "$TARGETS" == "" ]; then
			help
			#exit 1
		fi
        install
		;;
	* )	;;
esac
