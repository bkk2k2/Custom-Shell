#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

bool stopsig = false;

void handler(int sig){
	stopsig = true;
	return;
}

pair<int, vector<vector<string>>> tokenize(string line){

	vector<vector<string>> finalans; vector<string> ans; string curr = ""; int mode = 0, n = line.length();

	bool flag = false;

	for(int i=0; i<n; i++){

		if(flag){
			if(line[i] == '"') flag = false;
			else curr += line[i];
		}
		else{
			if(line[i] == '&'){
				if(i == n-1) mode = 1;
				else if((i == n-2) || (line[i+2] == ' ')) mode = 2;
				else mode = 3;
	
				if(curr != "") ans.push_back(curr);
				if(ans.size()) finalans.push_back(ans);
				ans.clear(); curr = "";
				i += mode; continue;
			}
			else if(line[i] == ' ') {ans.push_back(curr); curr = "";}
			else if(line[i] == '"') flag = true;
			else curr += line[i];
		}
	}

	if(curr != "") ans.push_back(curr);
	if(ans.size()) finalans.push_back(ans);

	return {mode, finalans};
}

int main(){
	string line; vector<vector<string>> fulltokens; vector<string> tokens; int mode; vector<pid_t> bgprc; vector<pid_t> fgprc;

	signal(SIGINT, handler);

	if(stopsig){
		int k, w;
		for(int i=0; i<fgprc.size(); i++) {k = kill(fgprc[i], SIGTERM); w = waitpid(fgprc[i], NULL, 0);}
		fgprc.clear();
	}

	while(1){
		line = ""; fulltokens.clear(); tokens.clear();
		cout << "$ ";
		getline(cin, line);

		pair<int, vector<vector<string>>> p = tokenize(line);
		mode = p.first; fulltokens = p.second; p = {};
		//cout << mode << " " << fulltokens.size() << endl;

		for(int ind = 0; ind < fulltokens.size(); ind++){
			if(stopsig){
				stopsig = false;
				break;
			}

			tokens.clear(); tokens = fulltokens[ind];

			int s = tokens.size(); if(s == 0) continue;

			if(tokens[0] == "cd"){

				if((s != 2) || (chdir(tokens[1].c_str()) != 0)){
					cout << "Incorrect command" << endl;
					continue;
				}

				continue;
			}

			if(tokens[0] == "exit"){
				int k, w;

				for(int i=0; i<bgprc.size(); i++) {k = kill(bgprc[i], SIGTERM); w = waitpid(bgprc[i], NULL, 0);}
				for(int i=0; i<fgprc.size(); i++) {k = kill(fgprc[i], SIGTERM); w = waitpid(fgprc[i], NULL, 0);}

				fulltokens.clear(); tokens.clear(); fgprc.clear(); bgprc.clear();
				exit(0);
			}

			pid_t rc = fork();

			if(rc < 0){
				cout << "Fork Failed!" << endl;
				exit(1);
			}
			else if(rc == 0){
				char *myargs[s+1];

				for(int i=0; i<s; i++) myargs[i] = &tokens[i][0];
				myargs[s] = NULL;

				execvp(myargs[0], myargs);

				cout << "Incorrect command" << endl;

				exit(1);
			}
			else{

				vector<pid_t> newbgprc;

				for(int i=0; i<bgprc.size(); i++){
					if(waitpid(bgprc[i], NULL, WNOHANG)) cout << "Background Process Finished" << endl;
					else newbgprc.push_back(bgprc[i]);
				}

				bgprc = newbgprc; newbgprc.clear();

				int wc;
				if((mode == 0) || (mode == 2)) wc = wait(NULL);
				else if(mode == 1){

					if(bgprc.size() == 0) setpgid(rc, 0);
					else setpgid(rc, getpgid(bgprc[0]));

					bgprc.push_back(rc);
				}
				else fgprc.push_back(rc);
			}
		}

		for(int i=0; i<fgprc.size(); i++){
			int w = waitpid(fgprc[i], NULL, 0);
		}

		fgprc.clear();
	}
}