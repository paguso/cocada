#!/bin/sh

help() {
	echo "COCADA Collection of Algorithms and DAta structures"
    echo "(c) 2016- Paulo Fonseca"
	echo ""
}

CWD=`pwd`
CPM_DIR=cpm
DEFAULT_INSTALL_DIR="$HOME/.cocada"
INSTALL_DIR=$DEFAULT_INSTALL_DIR
BASH_PROFILE="$HOME/.bash_profile"
REMOTE_REPO="https://github.com/paguso/cocada"

config_install_dir() {
    echo "Enter the install destination, or just press Return for the default '$INSTALL_DIR':"
    read ANS
    if [ ! "$ANS" = "" ] 
	then
        INSTALL_DIR=$ANS 
    fi 
    if [ -d $INSTALL_DIR ]
    then
		echo ""
        echo "WARNING: Installation directory $INSTALL_DIR already exists!"
        echo "Proceeding will PERMANENTLY OVERWRITE its contents."
        echo "Continue anyway? (Type \"Yes\" to confirm or anything else to abort)"
        read ANS
        if [ "$ANS" = "Yes" ] || [ "$ANS" = "yes" ]
        then
            echo "Overwriting $INSTALL_DIR"
			rm -rf $INSTALL_DIR/*
        else
			echo 
            echo "Aborting installation."
            exit 1
        fi
    fi
	INSTALL_DIR=`readlink -f $INSTALL_DIR`
    mkdir -p $INSTALL_DIR
    mkdir -p $INSTALL_DIR/bin
    mkdir -p $INSTALL_DIR/resources
}


write_env_setup() {
	ENV_SCRIPT=$INSTALL_DIR/cocadaenv.sh
	echo "#!/bin/sh" > $ENV_SCRIPT
	echo "# COCADA environment setup" >> $ENV_SCRIPT
	echo "export COCADA_HOME=\"$INSTALL_DIR\"" >> $ENV_SCRIPT
	echo "case \":\${PATH}:\" in" >> $ENV_SCRIPT
    echo "*:\"\$COCADA_HOME/bin\":*)" >> $ENV_SCRIPT
    echo "	;;" >> $ENV_SCRIPT
    echo "*)" >> $ENV_SCRIPT
    echo "	export PATH=\"\$COCADA_HOME/bin:\$PATH\"" >> $ENV_SCRIPT
    echo "	;;" >> $ENV_SCRIPT
	echo "esac" >> $ENV_SCRIPT
}


patch_profile() {
	write_env_setup
	echo ""
	echo "COCADA needs the environment variable \$CODADA_HOME to be defined and added to the PATH"
	case $SHELL in
	*"bash" )
		echo "It seems your system uses the bash shell. Great!"
		echo "I can patch the configuration file $HOME/.bash_profile"
		echo "(Don't worry, I'll backup the original file)"
		echo "Proceed? (Y)es | (N)o "
		read ANS 
		if [ "$ANS" = "Y" ] || [ "$ANS" = "y" ] || [ "$ANS" = "" ]; then
			if [ -e $HOME/.bash_profile ]
			then
				doit=false 
				if ! grep -q ". $ENV_SCRIPT" $BASH_PROFILE 
				then 
					doit=true
				else
					echo ""
					echo "$BASH_PROFILE already contains the following line:"
					grep -n ". $ENV_SCRIPT" $BASH_PROFILE
				fi
				if [ "$doit" = true ]
				then
					cp $BASH_PROFILE $BASH_PROFILE.cocada_bkp.`date +%Y%m%d%H%M`
					echo "" >> $BASH_PROFILE
					echo "# COCADA environment setup" >> $BASH_PROFILE
					echo ". $ENV_SCRIPT" >> $BASH_PROFILE
				fi 
			fi
			echo "Done."
		else 
			echo "OK, but please make the appropriate changes to your configuration files"
		fi
		;;
	* ) 
		echo "Please make the appropriate changes to your configuration files"
		;;
	esac
}


config_repo() {
	DEFAULT_LOCAL_REPO=$INSTALL_DIR/cocada
	LOCAL_REPO_CONF=$INSTALL_DIR/cocadareg.conf
	USE_CWD_REPO=false
	echo ""
	echo "Configuring COCADA repository"
	echo "COCADA needs to configure a git repository from which to pull source files."
	if [ -d $CWD/.git ] && [ -d $CWD/cocada ]
	then
		echo "It seems that the current directory $CWD"
		echo "is a COCADA git repository. You can choose to:"
		echo "1. Use the current directory as local repository; or"
		echo "2. Clone a separate local repository inside the installation directory. (default)"
		echo "Please, choose your option (Type '1' or '2')"
		read ANS
		if [ "$ANS" = "1" ] 
		then
			USE_CWD_REPO=true
		fi
	else 
		echo "A local COCADA repository will be cloned inside the installation directory."
	fi
	LOCAL_REPO=""
	if [ "$USE_CWD_REPO" = true ]
	then
		LOCAL_REPO=$CWD 
	else
		LOCAL_REPO=$DEFAULT_LOCAL_REPO
		if [ -d $LOCAL_REPO ]
		then
			rm -rf $LOCAL_REPO
		fi
		git clone $REMOTE_REPO $LOCAL_REPO
	fi
	echo 
	echo $LOCAL_REPO > $LOCAL_REPO_CONF
	echo "Done configuring COCADA repository."
}


install_cpm() {
	echo "Installing CPM"
	cd $LOCAL_REPO/cpm
	make release
	cp $LOCAL_REPO/cpm/build/release/cpm $INSTALL_DIR/bin
	cp -r $LOCAL_REPO/cpm/resources/* $INSTALL_DIR/resources
	echo "Done installing CPM"
}


install() {
	config_install_dir
	config_repo
	install_cpm
	patch_profile 
}




if [ $# -lt 1 ] 
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
		"help" ) 
			if [ ! "$NOWWHAT" = "op" ]
			then
				help
				exit 1
			fi
			help
			exit 0
			;;
		"install" ) 
			if [ ! "$NOWWHAT" = "op" ]
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
	"install" )	
		if [ "$TARGETS" = "" ]
		then
			help
			#exit 1
		fi
        install
		;;
	* )	;;
esac
