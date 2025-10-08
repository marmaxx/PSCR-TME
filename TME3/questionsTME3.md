# Questions - TME 3 : Threads

Instructions : copiez vos réponses dans ce fichier (sous la question correspondante). A la fin de la séance, commitez vos réponses.

## Question 1.

```
cd build-release && ./TME3 ../WarAndPeace.txt freqstd && ./TME3 ../WarAndPeace.txt freqstdf && ./TME3 ../WarAndPeace.txt freq && check.sh *.freq


traces pour les 3 modes, invocation a check qui ne rapporte pas d'erreur
```

```
Preparing to parse ../WarAndPeace.txt (mode=freqstd N=4), containing 3235342 bytes
Total runtime (wall clock) : 289 ms
Preparing to parse ../WarAndPeace.txt (mode=freqstdf N=4), containing 3235342 bytes
Total runtime (wall clock) : 288 ms
Preparing to parse ../WarAndPeace.txt (mode=freq N=4), containing 3235342 bytes
Total runtime (wall clock) : 298 ms
All files are identical
```
## Question 2.
```
start vaut 0

end vaut file_size

Code des lambdas :

code du lambda pour freqstdf et freq : 
[&](const std::string& word) {
    total_words++;
    um[word]++;
}
=> passage par référence pour tous 
=> Elle accède à total_words et à um
```

Accès identifiés :

## Question 3.

```
Preparing to parse ../WarAndPeace.txt (mode=freqstdf N=4), containing 3235342 bytes
Total runtime (wall clock) : 302 ms
Preparing to parse ../WarAndPeace.txt (mode=partition N=4), containing 3235342 bytes
Total runtime (wall clock) : 292 ms
```

## Question 4.


```
On a une segmentation fault car lorsque la table de hachage veut faire un redimensionement, hors la classe unordered_map n'est pas thread-safe, un autre thread peut accéder à une mémoire surement déplacée

Si on commente les join, on a :
Preparing to parse ../WarAndPeace.txt (mode=mt_naive N=4), containing 3235342 bytes
libc++abi: terminating
zsh: abort      ./TME3 ../WarAndPeace.txt mt_naive
```
## Question 5. 

```
peut être plus tard
```
## Question 6.

```
Total_words va maintenant retourner la même valeur mais toujours segmentation fault
```
## Question 7. 

```
peut être plus tard
```

## Question 8.

```
Preparing to parse ../WarAndPeace.txt (mode=mt_mutex N=4), containing 3235342 bytes
Total runtime (wall clock) : 210 ms
Files freq.freq and mt_atomic.freq differ
Files freq.freq and mt_naive.freq differ
Files freqstd.freq and mt_atomic.freq differ
Files freqstd.freq and mt_naive.freq differ
Files freqstdf.freq and mt_atomic.freq differ
Files freqstdf.freq and mt_naive.freq differ
Files mt_atomic.freq and mt_mutex.freq differ
Files mt_atomic.freq and partition.freq differ
Files mt_mutex.freq and mt_naive.freq differ
Files mt_naive.freq and partition.freq differ
```
## Question 9. 

```
peut être plus tard
```

## Question 10.

```
Preparing to parse ../WarAndPeace.txt (mode=mt_mutex N=4), containing 3235342 bytes
Total runtime (wall clock) : 212 ms
Files freq.freq and mt_atomic.freq differ
Files freq.freq and mt_naive.freq differ
Files freqstd.freq and mt_atomic.freq differ
Files freqstd.freq and mt_naive.freq differ
Files freqstdf.freq and mt_atomic.freq differ
Files freqstdf.freq and mt_naive.freq differ
Files mt_atomic.freq and mt_hashes.freq differ
Files mt_atomic.freq and mt_mutex.freq differ
Files mt_atomic.freq and partition.freq differ
Files mt_hashes.freq and mt_naive.freq differ
Files mt_mutex.freq and mt_naive.freq differ
Files mt_naive.freq and partition.freq differ
Maintenant la table de hachage, étant propre à tous les threads, n'a plus accès enc concurrence, c'est lorque tous les threads ont terminés que le thread principal va faire une union sur tous les autres threads
```