LJS
=====

## INSTALL

1. Environment
   1. Ubuntu 12.04
2. JSdep
   1. Follow instructions in [there](https://github.com/ChunghaSung/JSdep) and make JSdep work.
   2. Please put the dep result generated by JSdep to `./raw-data/yourprojectname/info/dep.txt`.
3. LJS
   1. As we developed LJS basing on Artemis-2.0.0, follow the instructions in INSTALL file. 

## USAGE

Basic commond

`./LJS file://yourproject_path/index.html`

1. -i 
   1. iteration numer
   2. `-i 10` means that generate a model and 9 test sequences
2. -m
   1. max length
   2. `-m 100` means generate a model whose bound (test sequence length) is 99
   3. Bound = max length - 1
3. -w
   1. If the current event depends on the previous event, give it a weight `w`
   2. `-w 0.7` means the weight value is 0.7
   3. LJS uses random selection strategy to select the next event. If use `-w` paramenter, LJS will use weighted strategy to select the next event. 
4. -a
   1. abstract or not
   2. If add `-a` parameter, LJS will abstract the model.

For example, the command that test a web application with max test sequence length 99, 9 test sequnces, random selection strategy and abstraction is `LJS file://yourproject_path/index.html -i 10 -m 100 -a`

All parameters applied in `Artemis` are also applicable in `LJS`.



  



