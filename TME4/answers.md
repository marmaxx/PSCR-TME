# TME4 Answers

Tracer vos expériences et conclusions dans ce fichier.

Le contenu est indicatif, c'est simplement la copie rabotée d'une IA, utilisée pour tester une version de l'énoncé.
On a coupé ses réponses complètes (et souvent imprécises voire carrément fausses, deadlocks etc... en Oct 2025 les LLM ont encore beaucoup de mal sur ces questions, qui demandent du factuel et des mesures, et ont de fortes tendances à inventer).
Cependant on a laissé des indications en particulier des invocations de l'outil possibles, à adapter à votre code.

## Question 1: Baseline sequential

### Measurements (Release mode)

**Resize + pipe mode:**
```
./build/TME4 -m resize -i input_images -o output_images

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'resize', nthreads 4
Thread 0x1f3cca0c0 (main): 2840 ms CPU
Total runtime (wall clock): 2949 ms
Memory usage: Resident: 531 MB, Peak: 531 MB
Total CPU time across all threads: 2840 ms

./build/TME4 -m pipe -i input_images -o output_images

Image resizer starting with input folder 'input_images', output folder 'output_images', mode 'pipe', nthreads 4
Thread 0x16b6e7000 (treatImage): 2826 ms CPU
Thread 0x1f3cca0c0 (main): 19 ms CPU
Total runtime (wall clock): 2953 ms
Memory usage: Resident: 522 MB, Peak: 522 MB
Total CPU time across all threads: 2845 ms

On a une différence entre le temps CPU et le temps réel car on ne tient pas en compte les possibles attentes dans le temps CPU (attente pour un accès disque notamment)

Le pic de RAM pourrait venir du fait que plusieurs images sont chargées en même temps 
```



## Question 2: Steps identification

4 étapes du code : 
``` 
pr::findImageFiles(opts.inputFolder, [&](const std::filesystem::path& file) {
            QImage original = pr::loadImage(file);
            if (!original.isNull()) {
                QImage resized = pr::resizeImage(original);
                std::filesystem::path outputFile = opts.outputFolder / file.filename();
                pr::saveImage(resized, outputFile);
            }
        });

I/O-bound: recherche, chargement, sauvegarde
CPU-bound: redimensionnement

parallelisable a priori ? 

Il vaut mieux pas utiliser de concurrence par rapport aux nombres de coeurs. Le système risque de surcharger (I/O en attente donc plus lent peut être)
```
## Question 3: BoundedBlockingQueue analysis

```
Nombre arbitraire de consommateurs et de producteurs car présence de wait et notify.
Bloquante si pleine et essaie de push et si vide et essaye de pop.
La lambda va vérifier si la taille est soit différente de 0 si si elle est inférieure à la taille. 
C'est du sucre syntaxique pour ne pas utiliser de while.
```
## Question 4: Pipe mode study

FILE_POISON ...
```
FILE_POISON est définie dans Tasks.h.
Dans le main, on push ce fichier après avoir push tous les autres rencontrés.
Lorsque le thread de traitement tombe dessus il sort de la boucle while.
Cette constante sert à indiquer à la queue que tout a été traité.
Order/invert :
2 et 3 ? non car pas la capacité nécessaire donc sans pop on a un deadlock
3 et 4 ? non car sinon on ne traite pas tous les fichiers
4 et 5 ? non (deadlock)
L'ordre est donc imposé
```
## Question 5: Multi-thread pipe_mt

Implement pipe_mt mode with multiple worker threads.

For termination, ... poison pills...
```
Bien penser à insérer n poisons
```

Measurements:
- N=1: 
```
./build/TME4 -m pipe_mt -n 1 -i input_images -o output_images

Thread 0x16daa3000 (treatImage): 2836 ms CPU
Thread 0x1f3cca0c0 (main): 26 ms CPU
Total runtime (wall clock): 2989 ms
Memory usage: Resident: 519 MB, Peak: 519 MB
Total CPU time across all threads: 2862 ms
```
- N=2: 
```
./build/TME4 -m pipe_mt -n 2 -i input_images -o output_images

Thread 0x16b1ff000 (treatImage): 1420 ms CPU
Thread 0x16b173000 (treatImage): 1536 ms CPU
Thread 0x1f3cca0c0 (main): 18 ms CPU
Total runtime (wall clock): 1632 ms
Memory usage: Resident: 703 MB, Peak: 704 MB
Total CPU time across all threads: 2974 ms
```
- N=4: 
```
./build/TME4 -m pipe_mt -n 4 -i input_images -o output_images

Thread 0x16de9f000 (treatImage): 701 ms CPU
Thread 0x16dfb7000 (treatImage): 772 ms CPU
Thread 0x16df2b000 (treatImage): 791 ms CPU
Thread 0x16e043000 (treatImage): 840 ms CPU
Thread 0x1f3cca0c0 (main): 18 ms CPU
Total runtime (wall clock): 905 ms
Memory usage: Resident: 847 MB, Peak: 941 MB
Total CPU time across all threads: 3122 ms
```
- N=8: 
```
./build/TME4 -m pipe_mt -n 8 -i input_images -o output_images

Thread 0x16ddc7000 (treatImage): 423 ms CPU
Thread 0x16d9f3000 (treatImage): 454 ms CPU
Thread 0x16dcaf000 (treatImage): 454 ms CPU
Thread 0x16dc23000 (treatImage): 470 ms CPU
Thread 0x16db0b000 (treatImage): 486 ms CPU
Thread 0x16da7f000 (treatImage): 529 ms CPU
Thread 0x16dd3b000 (treatImage): 557 ms CPU
Thread 0x16db97000 (treatImage): 564 ms CPU
Thread 0x1f3cca0c0 (main): 18 ms CPU
Total runtime (wall clock): 742 ms
Memory usage: Resident: 755 MB, Peak: 997 MB
Total CPU time across all threads: 3955 ms
```

- N=16: 
```
./build/TME4 -m pipe_mt -n 16 -i input_images -o output_images

Thread 0x16db13000 (treatImage): 190 ms CPU
Thread 0x16dc2b000 (treatImage): 200 ms CPU
Thread 0x16e22f000 (treatImage): 221 ms CPU
Thread 0x16e2bb000 (treatImage): 198 ms CPU
Thread 0x16de5b000 (treatImage): 208 ms CPU
Thread 0x16dfff000 (treatImage): 193 ms CPU
Thread 0x16e1a3000 (treatImage): 243 ms CPU
Thread 0x16db9f000 (treatImage): 247 ms CPU
Thread 0x16dcb7000 (treatImage): 249 ms CPU
Thread 0x16df73000 (treatImage): 248 ms CPU
Thread 0x16ddcf000 (treatImage): 288 ms CPU
Thread 0x16e08b000 (treatImage): 277 ms CPU
Thread 0x16e347000 (treatImage): 274 ms CPU
Thread 0x16dd43000 (treatImage): 312 ms CPU
Thread 0x16dee7000 (treatImage): 336 ms CPU
Thread 0x16e117000 (treatImage): 351 ms CPU
Thread 0x1f3cca0c0 (main): 18 ms CPU
Total runtime (wall clock): 699 ms
Memory usage: Resident: 830 MB, Peak: 1.24 GB
Total CPU time across all threads: 4053 ms
```

- N=32: 
```
./build/TME4 -m pipe_mt -n 32 -i input_images -o output_images

Thread 0x16db13000 (treatImage): 190 ms CPU
Thread 0x16dc2b000 (treatImage): 200 ms CPU
Thread 0x16e22f000 (treatImage): 221 ms CPU
Thread 0x16e2bb000 (treatImage): 198 ms CPU
Thread 0x16de5b000 (treatImage): 208 ms CPU
Thread 0x16dfff000 (treatImage): 193 ms CPU
Thread 0x16e1a3000 (treatImage): 243 ms CPU
Thread 0x16db9f000 (treatImage): 247 ms CPU
Thread 0x16dcb7000 (treatImage): 249 ms CPU
Thread 0x16df73000 (treatImage): 248 ms CPU
Thread 0x16ddcf000 (treatImage): 288 ms CPU
Thread 0x16e08b000 (treatImage): 277 ms CPU
Thread 0x16e347000 (treatImage): 274 ms CPU
Thread 0x16dd43000 (treatImage): 312 ms CPU
Thread 0x16dee7000 (treatImage): 336 ms CPU
Thread 0x16e117000 (treatImage): 351 ms CPU
Thread 0x1f3cca0c0 (main): 18 ms CPU
Total runtime (wall clock): 699 ms
Memory usage: Resident: 830 MB, Peak: 1.24 GB
Total CPU time across all threads: 4053 ms
```

- N=64: 
```
./build/TME4 -m pipe_mt -n 64 -i input_images -o output_images

Thread 0x16cc13000 (treatImage): 0 ms CPU
Thread 0x16cd2b000 (treatImage): 0 ms CPU
Thread 0x16ce43000 (treatImage): 0 ms CPU
Thread 0x16cecf000 (treatImage): 0 ms CPU
Thread 0x16cdb7000 (treatImage): 0 ms CPU
Thread 0x16d073000 (treatImage): 0 ms CPU
Thread 0x16d0ff000 (treatImage): 0 ms CPU
Thread 0x16d18b000 (treatImage): 0 ms CPU
Thread 0x16d217000 (treatImage): 0 ms CPU
Thread 0x16d2a3000 (treatImage): 0 ms CPU
Thread 0x16d32f000 (treatImage): 0 ms CPU
Thread 0x16ba07000 (treatImage): 0 ms CPU
Thread 0x16d4d3000 (treatImage): 0 ms CPU
Thread 0x16d81b000 (treatImage): 0 ms CPU
Thread 0x16d447000 (treatImage): 0 ms CPU
Thread 0x16bbab000 (treatImage): 0 ms CPU
Thread 0x16d8a7000 (treatImage): 0 ms CPU
Thread 0x16bb1f000 (treatImage): 0 ms CPU
Thread 0x16d3bb000 (treatImage): 0 ms CPU
Thread 0x16d5eb000 (treatImage): 0 ms CPU
Thread 0x16d703000 (treatImage): 0 ms CPU
Thread 0x16d933000 (treatImage): 0 ms CPU
Thread 0x16d78f000 (treatImage): 0 ms CPU
Thread 0x16b863000 (treatImage): 27 ms CPU
Thread 0x16c727000 (treatImage): 32 ms CPU
Thread 0x16cafb000 (treatImage): 31 ms CPU
Thread 0x16ba93000 (treatImage): 38 ms CPU
Thread 0x16c2c7000 (treatImage): 33 ms CPU
Thread 0x16c1af000 (treatImage): 49 ms CPU
Thread 0x16c46b000 (treatImage): 45 ms CPU
Thread 0x16d9bf000 (treatImage): 47 ms CPU
Thread 0x16b74b000 (treatImage): 37 ms CPU
Thread 0x16c097000 (treatImage): 36 ms CPU
Thread 0x16bf7f000 (treatImage): 78 ms CPU
Thread 0x16bc37000 (treatImage): 50 ms CPU
Thread 0x16c123000 (treatImage): 52 ms CPU
Thread 0x16cfe7000 (treatImage): 82 ms CPU
Thread 0x16cb87000 (treatImage): 71 ms CPU
Thread 0x16c8cb000 (treatImage): 86 ms CPU
Thread 0x16c69b000 (treatImage): 102 ms CPU
Thread 0x16bd4f000 (treatImage): 93 ms CPU
Thread 0x16c353000 (treatImage): 89 ms CPU
Thread 0x16cf5b000 (treatImage): 103 ms CPU
Thread 0x16c583000 (treatImage): 85 ms CPU
Thread 0x16b97b000 (treatImage): 88 ms CPU
Thread 0x16b8ef000 (treatImage): 98 ms CPU
Thread 0x16c7b3000 (treatImage): 89 ms CPU
Thread 0x16c00b000 (treatImage): 131 ms CPU
Thread 0x16b7d7000 (treatImage): 142 ms CPU
Thread 0x16ca6f000 (treatImage): 106 ms CPU
Thread 0x16c4f7000 (treatImage): 151 ms CPU
Thread 0x16c957000 (treatImage): 106 ms CPU
Thread 0x16c23b000 (treatImage): 143 ms CPU
Thread 0x16c60f000 (treatImage): 160 ms CPU
Thread 0x16d677000 (treatImage): 136 ms CPU
Thread 0x16be67000 (treatImage): 158 ms CPU
Thread 0x16cc9f000 (treatImage): 151 ms CPU
Thread 0x16bddb000 (treatImage): 168 ms CPU
Thread 0x16bef3000 (treatImage): 153 ms CPU
Thread 0x16c83f000 (treatImage): 138 ms CPU
Thread 0x16c3df000 (treatImage): 175 ms CPU
Thread 0x16bcc3000 (treatImage): 170 ms CPU
Thread 0x16d55f000 (treatImage): 185 ms CPU
Thread 0x16c9e3000 (treatImage): 214 ms CPU
Thread 0x1f3cca0c0 (main): 19 ms CPU
Total runtime (wall clock): 707 ms
Memory usage: Resident: 748 MB, Peak: 1.69 GB
Total CPU time across all threads: 4147 ms
```
Best: 16 threads (par rapport au temps réel, + on a de I/O, + elles sont lentes)

## Question 6: TaskData struct

```cpp
struct TaskData {
    QImage image;
    std::filesystem::path filename;
};
```

Fields: QImage ??? for the image data, ...

Use ??? for QImage, because ...

TASK_POISON: ...def...

## Question 7: ImageTaskQueue typing

pointers vs values

Choose BoundedBlockingQueue<TaskData???> as consequence

## Question 8: Pipeline functions

Implement reader, resizer, saver in Tasks.cpp.

mt_pipeline mode: Creates threads for each stage, with configurable numbers.

Termination: Main pushes the appropriate number of poisons after joining the previous stage.

Measurements: 
```
./build/TME4 -m mt_pipeline -i input_images -o output_images
...
```


## Question 9: Configurable parallelism

Added nbread, nbresize, nbwrite options.


Timings:
- 1/1/1 (default): 
```
./build/TME4 -m mt_pipeline -i input_images -o output_images
...
```
- 1/4/1: 
```
./build/TME4 -m mt_pipeline --nbread 1 --nbresize 4 --nbwrite 1 -i input_images -o output_images
```

- 4/1/1: 
```
./build/TME4 -m mt_pipeline --nbread 4 --nbresize 1 --nbwrite 1 -i input_images -o output_images
```
... autres configs

Best config: 
interprétation

## Question 10: Queue sizes impact


With size 1: 
```
./build/TME4 -m pipe_mt -n 2 --queue-size 1 -i input_images -o output_images
...
```

With size 100: 
```
./build/TME4 -m pipe_mt -n 2 --queue-size 100 -i input_images -o output_images
...
```

impact

Complexity: 


## Question 11: BoundedBlockingQueueBytes

Implemented with byte limit.

mesures

## Question 12: Why important

Always allow push if current_bytes == 0, ...

Fairness: ...

## Bonus

