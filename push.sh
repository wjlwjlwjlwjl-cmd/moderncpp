collect_info(){
	echo please input the repository:
	read REPOSITORY
	echo please input the dir or file:
	read DIR_FILE
	echo please input the commit message:
	read COMMIT_MESSAGE
}

enter_dir(){
	cd /c
	cd Gitfiles
	cd $REPOSITORY
}

push_to_remote(){
	git add $DIR_FILE
	git commit -m "$COMMIT_MESSAGE"
	git push https://gitee.com/wangs-joyful-home/moderncpp.git
	if [ $? -eq 0 ]; then
		echo -e "\033[32mfirst part to gitee, done...\033[0m"
	else
		echo -e "\033[31msomething bad happened way to gitee, please check the output\033[0m"
	fi
	git push https://github.com/wjlwjlwjlwjl-cmd/moderncpp.git
	if [ $? -eq 0 ]; then
		echo -e "\033[32msecond part to github, done...\033[0m"
		echo -e "\033[32mnothing left...\033[0m"
	else
		echo -e "\033[31msomething bad happened way to github, please check the output\033[0m"
	fi
}

collect_info

enter_dir
push_to_remote
