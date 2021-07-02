# jhin-lang  
  
 ## [Installation](#anc_0)  
 ## [Developer tutorial](#anc_1)
 ## [Architecture](#anc_2)
 ## [Be a contributor](#anc_3)
 
  
<h2 name = "anc_0">Installation</h2>
<h2 name = "anc_1">Developer tutorial</h2>
<h2 name = "anc_2">Architecture</h2>
<h2 name = "anc_3">Be a contributor</h2>

Project built in C++17    
dependency：  
- `Python3`  
- `matplotlib`  
  
How to deploy：  
1. [Building Clang and Working with the Code][0]  
2. go to llvm-project/llvm/examples/Kaleidoscope and add `add_subdirectory(Jhin)` to CMakeLists.txt    
3. `git submodule add https://<token>@github.com/elfcoda/jhin.git Jhin`   
    You should create a personal access `<token>` to use in place of a password with the command line or with the API.  
    [how to get your token][1]  
  
  
  
  
  
  
  
  
[0]: https://clang.llvm.org/get_started.html
[1]: https://docs.github.com/en/github/authenticating-to-github/keeping-your-account-and-data-secure/creating-a-personal-access-token  
