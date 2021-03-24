#define FORI(i,m) for(int i = 0; i<m;i++) 
#include <vector>
#include <map>
#include <queue>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std;

/*
     variables globales
*/

//variable donde se almacenan los tiempos de cada maquina para cada tarea
vector<vector<float>> taskTimes;
//variable donde se almacenan los trabajos
queue<vector<int>> jobs;
//colas de trabajo de cada maquina
vector<queue<pair<float, pair<int, int>>>>machines;
//variable que almacena el numero de trabajos realizados en toda la ejecucion
int jobsNumber = 1;
// variable que no permite que los hilos usen la misma variable al mismo tiempo;
mutex mtx;
//variable que indica cuando el hilo de entrada ya terminó
bool finish = true;
/*
//variable condicional que ayuda a dar inicio a los hilos en el timpo correcto
condition_variable cv;
*/
//variable que facilita al usuario la entrada de datos
string mensaje ="";
//variable que indica cuantas maquinas han terminado
 int machinesFinished;
 
/*
     Declaracion de funciones
*/

/*
todas las funciones de lectura requieren un archivo de entrada, ya sea txt, data o in, este se especifica al correr el programa, si no se ingresa ningun nombre el archivo por default es el indicado en las condiciones
*/

/*
funcion que lee los tiempos de cada maquina en realizar una tarea
la lectura se realiza:
primero se leen 2 enteros (numero_de_maquinas numero_de_operaciones)
despues se leen los tiempos en que cada maquina se tarda en realizar cada tarea (en orden acendente, i.e 
 0-numero_de operaciones y de 0-numero de maquinas)
 solo se leen los tiempos, no es necesario ingresar los nombres de cada maquina ni los nombres de cada operacion
*/
void lecturaTareas(string nombreArchivo);

/*
funcion que lee los trabajos a realizar
se lee conforme cada trabajo esta encolado
la forma de lectura es:
primero se lee un entero(numero_de_trabajos) y luego por cada trabajo se lee
un entero que indica el numero de operacione de este trabajo y
despues con un entero se indica que operacion es, las operaciones se ordenan desde 1-numero_de_operaciones
*/
void lecturaTrabajos(string nombreArchivo);

/*
funcion que encola los trabajos en cada maquina;
*/
void analisisTrabajos();

/*
funcion que asigna trabajo a una maquina
*/
void asignarTrabajo(int machine, vector<int> job);

/*
funcion que calcula la mejor maquina disponible
*/
int buscarMaquina(const vector<int> job);

/*
funcion que calcula el tiempo que le falta a una maquina
*/
float tiempoRestante(queue<pair<float, pair<int, int>>>machine);

/*
funcion que simula el tiempo de ejecucion de las tareas
*/
void ejecutarTareas();

/*
funcion que agrega tareas a las ya obtenidas
*/
void aniadirTareas();



int main(){

     lecturaTareas("");
     lecturaTrabajos("");
     analisisTrabajos();
     
     //hilo que maneja la entrada de nuevas tareas
     thread newTareas(aniadirTareas);
     //hilo que maneja la ejecucion de tareas
     thread tareas(ejecutarTareas);
     newTareas.join();
     finish = newTareas.joinable();
     tareas.join();
    return 0;
}

void lecturaTareas(string nombreArchivo){
     if(nombreArchivo == "") nombreArchivo = "taskTimes.in";
     ifstream ent(nombreArchivo);
     int numMachines, numTask;
     ent >> numMachines>>numTask;
     machinesFinished = numMachines+1;
     queue<pair<float, pair<int, int>>> tasksM;
     vector<queue<pair<float, pair<int, int>>>>nMachines(numMachines, tasksM);
     machines=nMachines;
     while(numMachines--){
          int task = numTask;
          vector<float> times;
          while(task--){
               float time;
               ent>>time;
               times.push_back(time);
          }
          taskTimes.push_back(times);
     }
}
void lecturaTrabajos(string nombreArchivo){
     if(nombreArchivo=="") nombreArchivo="jobs.in";
     ifstream ent(nombreArchivo);
     int numJobs;
     ent>>numJobs;
     while(numJobs--){
          int numTask;
          ent>>numTask;
          vector<int>tasks;
          while(numTask--){
               int task;
               ent>>task;
               tasks.push_back(task);
          }
          jobs.push(tasks);
     }
}

float tiempoRestante(queue<pair<float, pair<int, int>>>machine){
     float time =0;
     FORI(i, machine.size()){
          time+=machine.front().first;
          machine.pop();
     }
     return time;
}

int buscarMaquina(const vector<int> job){    
     mensaje +="job now: ";
     for(auto i : job) mensaje += " " + to_string(i);
     mensaje +="\n";
     float timeNow;
     float time=0;
     int machineSelected;
     FORI(i, machines.size()){
          timeNow = 0.0;
          if(machines[i].size()>0&& machines[i].front().first>0){
               timeNow += tiempoRestante(machines[i]);
          }
          FORI(j,job.size()){
               if(timeNow >=time&& i!=0){
                    break;
               }
               timeNow += taskTimes[i][job[j]-1];
          }
          if(i==0){
                    time = timeNow;
                    machineSelected=i;
               }
          if(time >timeNow){
               time = timeNow;
               machineSelected = i;
          }
     }
     if(machines[machineSelected].front().first == 0.0) machinesFinished--;
     mensaje += "machine for this is : "+ to_string(machineSelected+1);
     return machineSelected;
 }

void asignarTrabajo(int machine, vector<int> job){
     FORI(i, job.size()){
          machines[machine].push(make_pair(taskTimes[machine][job[i]], make_pair(jobsNumber, job[i])));
     }
     jobsNumber++;
}

void analisisTrabajos(){
     while(!jobs.empty()){
          asignarTrabajo(buscarMaquina(jobs.front()), jobs.front());
          jobs.pop();
     }
}

void ejecutarTareas(){
     mensaje = "";
     while(machinesFinished<machines.size()||finish){
          system("clear");
          machinesFinished = 0;
          FORI(i, machines.size()){
               cout<<"maquina "<<i+1<<" : "<<machines[i].front().first<<" time left "<<" doing job: "<<machines[i].front().second.first
               <<" task: "<<machines[i].front().second.second<<endl;
               if(machines[i].front().first == 0.0) machinesFinished++;
          }
          //cout <<"machines free: "<<machinesFinished<<"valor finish "<<finish<<endl;
          cout <<mensaje<<endl;
          FORI(i, machines.size()){
               if(machines[i].front().first <1&&machines[i].front().first>0){
                    if(machines[i].size()==1){ 
                         machines[i].front().first=0;
                         continue;
                    }
                    float timeRest = machines[i].front().first;
                    machines[i].pop();
                    machines[i].front().first+=timeRest;
                   
               }
               if(machines[i].size()==1&& machines[i].front().first == 0.0){
                    machines[i].front()=make_pair(0.0, make_pair(0,0)); 
               }
               if(machines[i].front().first == 0.0&&machines[i].size()>1)machines[i].pop();
               if(machines[i].front().first>=1)machines[i].front().first-=1;
          }
           this_thread::sleep_for(chrono::milliseconds(1500));
     }
     system("clear");
      FORI(i, machines.size()){
               cout<<"maquina "<<i<<" : "<<machines[i].front().first<<"time left"<<"doing job:"<<machines[i].front().second.first
               <<"task: "<<machines[i].front().second.second<<endl;
          }
}

void aniadirTareas(){
     mtx.lock();
     int numTask; cin>>numTask;
     if(numTask==0) 
     mensaje+="Se termino la entrada de nuevas tareas ";
     mtx.unlock();
     while (numTask>0){
          vector<int> newJob;
          FORI(i, numTask){
               mtx.lock();
               mensaje = "ingrese la tarea No." + to_string(i)+" de"+to_string(numTask);
               int task; cin >>task;
               mtx.unlock();
               newJob.push_back(task);
          }
          mtx.lock();
          mensaje = "";
          asignarTrabajo(buscarMaquina(newJob), newJob);
          cin>>numTask;
          if(numTask==0) 
          mensaje="Se termino la entrada de nuevas tareas";
          mtx.unlock();
     }
}