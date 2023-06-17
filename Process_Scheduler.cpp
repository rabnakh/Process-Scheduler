/*
This is a simulation of the execution of processes by a tablet with a large memory,
one display, and one solid state drive. Each process will be described by its start time
and its process if followed by a sequence of resources requests. All the times are expressed in 
milliseconds.*/
#include <iostream>
#include <iomanip>
#include <string>
#include <queue>
using namespace std;
/*This is a struct to make the event queue*/
struct EventNode{
    int PID = 0;
    string resource = "";
    string interactionType = "";
    int completionTime = 0;
    EventNode* nextEventNode = nullptr;
};
/*This is a struct to make the process table*/
struct ProcessNode{
    int PID = 0;
    string state = "";
    int startTime = 0;
    int totalResourceRequests = 0;
    int currentResourceRequest = 0;
    ProcessNode* nextProcessNode = nullptr;
};
/*This is a struct to make the input table*/
struct InputNode{
    int PID = 0;
    string resource = "";
    int resourceRequestedTime = 0;
    InputNode* nextInputNode = nullptr;
    InputNode* downInputNode = nullptr;
};
/*This is the class of type simulation that will have the functions and varibles to
run the simulation*/
class Simulation{
    public:
    int GLOBAL_CLOCK = 0;
    double totalCORERequestedTime = 0.0;
    double totalSSDRequestedTime = 0.0;
    int totalCores = 0;
    int freeCores = 0;
    int totalCompletedProcesses = 0;
    int totalSSDAccesses = 0;
    bool SSDStatus = true;
    EventNode* EventListHead = nullptr;
    ProcessNode* ProcessTableHead = nullptr;
    InputNode* InputListHead = nullptr;
    queue <int> non_interactiveQueue;
    queue <int> interactiveQueue;
    queue <int> SSDQueue;
    /*This function will add an "index" node within a linked list. The index node
    will point to its unique list of resource requests.*/
    void AddInputStartNode(string rightString,int tempStartTime){
        InputNode* newInputIndexNode = new InputNode;
        newInputIndexNode->PID = stoi(rightString);
        newInputIndexNode->resource = "START";
        newInputIndexNode->resourceRequestedTime = tempStartTime;
        if(InputListHead == nullptr) InputListHead = newInputIndexNode;
        else{
            InputNode* inputIndexTraverser = InputListHead;
            while(inputIndexTraverser->nextInputNode)
                inputIndexTraverser = inputIndexTraverser->nextInputNode;
            inputIndexTraverser->nextInputNode = newInputIndexNode;
        }
    }
    /*This function will add a resource request node to the most recently created "index node"*/
    void AddInputResourceRequestNode(string leftString,string rightString){
        InputNode* newInputResourceRequest = new InputNode;
        newInputResourceRequest->resource = leftString;
        newInputResourceRequest->resourceRequestedTime = stoi(rightString);
        InputNode* inputTravsererToAddResourceRequest = InputListHead;
        while(inputTravsererToAddResourceRequest->nextInputNode)
            inputTravsererToAddResourceRequest = inputTravsererToAddResourceRequest->nextInputNode;
        while(inputTravsererToAddResourceRequest->downInputNode) 
            inputTravsererToAddResourceRequest = inputTravsererToAddResourceRequest->downInputNode;
        inputTravsererToAddResourceRequest->downInputNode = newInputResourceRequest;
    }
    /*This function will read the input fill and call the two previous two functions to add the nodes
    to the input table.*/
    void ReadInputFile(){
        string leftString = ""; string rightString = "";
        int tempStartTime = 0;
        while(cin >> leftString >> rightString){
            if(leftString == "END") return;
            if(leftString == "NCORES") totalCores = freeCores = stoi(rightString);
            else{
                if(leftString == "START") tempStartTime = stoi(rightString);
                else if(leftString == "PID") AddInputStartNode(rightString,tempStartTime);
                else AddInputResourceRequestNode(leftString,rightString);
            }
        }
    }
    /*This function will print the process table*/
    void PrintProcessTable(){
        cout << "Process Table:\n";
        if(!ProcessTableHead){
            cout << "There are no active processes.\n\n";
            return;
        }
        ProcessNode* processTraverser = ProcessTableHead;
        while(processTraverser){
            cout << "Process " << processTraverser->PID;
            cout << " is " << processTraverser->state << ".\n";
            processTraverser = processTraverser->nextProcessNode;
        }
        cout << endl;
    }
    /*This function will get the varible values of the front node in the event queue*/
    void TopEventFromQueue(int &PID,string &resource,int &requestedTime){
        PID = EventListHead->PID;
        resource = EventListHead->resource;
        requestedTime = EventListHead->completionTime;
    }
    /*This function will pop the front node from the event queue*/
    void PopEventFromEventQueue(){
        EventNode* PoppedEvent = EventListHead;
        EventListHead = EventListHead->nextEventNode;
        PoppedEvent->nextEventNode = nullptr;
    }
    /*This function will push a node to the event queue*/
    void PushEventToEventQueue(EventNode* newEventToPush){
        if(EventListHead == nullptr) EventListHead = newEventToPush;
        else{
            EventNode* traverserAdder = EventListHead;
            while(traverserAdder->nextEventNode) traverserAdder = traverserAdder->nextEventNode;
            traverserAdder->nextEventNode = newEventToPush;
        }
    }
    /*This function will sort the event queue*/
    void SortEventQueue(){
        if(EventListHead == nullptr) return;
        EventNode* traverser = EventListHead;
        EventNode* previous = nullptr;
        bool sorted = true;
        while(sorted){
            sorted = false;
            while(traverser->nextEventNode){
                if(traverser->completionTime > traverser->nextEventNode->completionTime){
                    sorted = true;
                    if(traverser == EventListHead){
                        EventListHead = EventListHead->nextEventNode;
                        traverser->nextEventNode = traverser->nextEventNode->nextEventNode;
                        EventListHead->nextEventNode = traverser;
                        previous = EventListHead;
                    }
                    else{
                        previous->nextEventNode = previous->nextEventNode->nextEventNode;
                        traverser->nextEventNode = traverser->nextEventNode->nextEventNode;
                        previous->nextEventNode->nextEventNode = traverser;
                        previous = previous->nextEventNode;
                    }
                }
                else{
                    previous = traverser;
                    traverser = traverser->nextEventNode;
                }
            }
            traverser = EventListHead;
            previous = nullptr;
        }
    }
    /*This function will set up the event queue with the correct start times and PID numbers*/
    void ScheduleArrivalTimes(){
        InputNode* inputIndexTraverser = InputListHead;
        while(inputIndexTraverser){
            EventNode* newEventToAdd = new EventNode;
            newEventToAdd->PID = inputIndexTraverser->PID;
            newEventToAdd->resource = inputIndexTraverser->resource;
            newEventToAdd->completionTime = inputIndexTraverser->resourceRequestedTime;
            PushEventToEventQueue(newEventToAdd);
            inputIndexTraverser = inputIndexTraverser->nextInputNode;
        }
    }
    /*This function will get the resouce request values from the input table for a given PID value*/
    void GetResourceAndResourceRequestTimeFromNextRequestinInputList(int PID, string &resourceRequested, int &requestedTime){
        ProcessNode* processTraverser = ProcessTableHead;
        while(processTraverser->PID != PID) processTraverser = processTraverser->nextProcessNode;
        int currentResourceRequest = processTraverser->currentResourceRequest;
        if(currentResourceRequest == processTraverser->totalResourceRequests){
            resourceRequested = "END";
            return;
        }
        InputNode* inputTravserer = InputListHead;
        while(inputTravserer->PID != PID) inputTravserer = inputTravserer->nextInputNode;
        inputTravserer = inputTravserer->downInputNode;
        for(int index = 0; index < currentResourceRequest;index++)
            inputTravserer = inputTravserer->downInputNode;
        resourceRequested = inputTravserer->resource;
        requestedTime = inputTravserer->resourceRequestedTime;
    }
    /*This function will add a newly arrived process to the process table*/
    void AddArrivalProcessToTable(int PID){
        ProcessNode* newProcessToAdd = new ProcessNode;
        newProcessToAdd->PID = PID;
        newProcessToAdd->state = "Arrival";
        newProcessToAdd->startTime = GLOBAL_CLOCK;
        InputNode* inputIndexTraverser = InputListHead;
        while(inputIndexTraverser->PID != PID)
            inputIndexTraverser = inputIndexTraverser->nextInputNode;
        inputIndexTraverser = inputIndexTraverser->downInputNode;
        while(inputIndexTraverser){
            newProcessToAdd->totalResourceRequests++;
            inputIndexTraverser = inputIndexTraverser->downInputNode;
        }
        if(!ProcessTableHead) ProcessTableHead = newProcessToAdd;
        else{
            ProcessNode* processTraverser = ProcessTableHead;
            while(processTraverser->nextProcessNode)
                processTraverser = processTraverser->nextProcessNode;
            processTraverser->nextProcessNode = newProcessToAdd;
        }
    }
    /*This function will update the update the state of a given PID in the process table*/
    void UpdateProcessState(int PID, string state){
        ProcessNode* processTraverser = ProcessTableHead;
        while(processTraverser->PID != PID)
            processTraverser = processTraverser->nextProcessNode;
        processTraverser->state = state;
    }
    /*This function will delete a terminated process from the process table*/
    void DeleteTerminatedProcess(int PID){
        ProcessNode* processTraverser = ProcessTableHead;
        if(ProcessTableHead->PID == PID){
            ProcessTableHead = ProcessTableHead->nextProcessNode;
            processTraverser->nextProcessNode = nullptr;
        }
        else{
            while(processTraverser->nextProcessNode->PID != PID)
                processTraverser = processTraverser->nextProcessNode;
            ProcessNode* tempProcess = processTraverser->nextProcessNode;
            processTraverser->nextProcessNode = processTraverser->nextProcessNode->nextProcessNode;
            tempProcess->nextProcessNode = nullptr;
        }
    }
    /*This function will increment the resource request counter for a given PID in the process table*/
    void IncrementOneToCurrentResourceRequestCounter(int PID){
        ProcessNode* processTraverser = ProcessTableHead;
        while(processTraverser->PID != PID) processTraverser = processTraverser->nextProcessNode;
        processTraverser->currentResourceRequest = processTraverser->currentResourceRequest + 1;
    }
    /*This function will request a core for a given PID. If all the cores are being used then
    the PID will be pushed to one of the interactive or non-interactive queueus*/
    void Core_Request(int resourceRequestedTime, int PID, string interactionType){
        if(freeCores > 0){
            freeCores--;
            totalCORERequestedTime += resourceRequestedTime;
            EventNode* newEventScheduled = new EventNode;
            newEventScheduled->completionTime = GLOBAL_CLOCK + resourceRequestedTime;
            newEventScheduled->PID = PID;
            newEventScheduled->resource = "CORE";
            PushEventToEventQueue(newEventScheduled);
            UpdateProcessState(PID,"RUNNING");
            IncrementOneToCurrentResourceRequestCounter(PID);
        }
        else{
            if(interactionType == "interactive"){
                interactiveQueue.push(PID);
                interactiveQueue.push(resourceRequestedTime);
            }
            else if(interactionType == "non_interactive"){
                non_interactiveQueue.push(PID);
                non_interactiveQueue.push(resourceRequestedTime);
            }
            UpdateProcessState(PID,"READY");
        }
    }
    /*This function will request the SSD for a given PID. If the SSD is being used then the PID
    will be pushed to the the SSD queue*/
    void SSD_Request(int requestedTime,int PID){
        totalSSDRequestedTime += requestedTime;
        if(SSDStatus == true){
            SSDStatus = false;
            EventNode* newEventScheduled = new EventNode;
            newEventScheduled->PID = PID;
            newEventScheduled->resource = "SSD";
            newEventScheduled->completionTime = GLOBAL_CLOCK + requestedTime;
            PushEventToEventQueue(newEventScheduled);
            IncrementOneToCurrentResourceRequestCounter(PID);
            totalSSDAccesses++;
        }
        else{
            SSDQueue.push(PID);
            SSDQueue.push(requestedTime);
        }
        UpdateProcessState(PID,"BLOCKED");
    }
    /*This function will request user interaction for a given PID*/
    void UserInteraction_Request(int requestedTime, int PID){
        EventNode* newEventScheduled = new EventNode;
        newEventScheduled->PID = PID;
        newEventScheduled->resource = "TTY";
        newEventScheduled->completionTime = GLOBAL_CLOCK + requestedTime;
        PushEventToEventQueue(newEventScheduled);
        UpdateProcessState(PID,"BLOCKED");
        IncrementOneToCurrentResourceRequestCounter(PID);
    }
    /*This function will release a PID from a CORE and make an SSD request.
    If one of the interactive queues are not empty then it will pop a PID from that queue
    and add it to the event queue with a status of CORE*/
    void Core_Release(int PID){
        if(!interactiveQueue.empty() || !non_interactiveQueue.empty()){
            EventNode* newEventScheduledFromQueue = new EventNode;
            newEventScheduledFromQueue->resource = "CORE";
            if(!interactiveQueue.empty()){
                newEventScheduledFromQueue->PID = interactiveQueue.front();
                interactiveQueue.pop();
                totalCORERequestedTime += interactiveQueue.front();
                newEventScheduledFromQueue->completionTime = GLOBAL_CLOCK + interactiveQueue.front();
                interactiveQueue.pop();
            }
            else{
                newEventScheduledFromQueue->PID = non_interactiveQueue.front();
                non_interactiveQueue.pop();
                totalCORERequestedTime += non_interactiveQueue.front();
                newEventScheduledFromQueue->completionTime = GLOBAL_CLOCK + non_interactiveQueue.front();
                non_interactiveQueue.pop();
            }
            PushEventToEventQueue(newEventScheduledFromQueue);
            UpdateProcessState(newEventScheduledFromQueue->PID,"RUNNING");
            IncrementOneToCurrentResourceRequestCounter(newEventScheduledFromQueue->PID);
        }
        else freeCores++;
        string resourceRequested = ""; int requestedTime = 0;
        GetResourceAndResourceRequestTimeFromNextRequestinInputList(PID,resourceRequested,requestedTime);
        if(resourceRequested == "SSD") SSD_Request(requestedTime,PID);
        else if(resourceRequested == "TTY") UserInteraction_Request(requestedTime,PID);
        else if(resourceRequested == "END"){
            UpdateProcessState(PID,"TERMINATED");
            cout << "Process " << PID << " terminates at time " << GLOBAL_CLOCK << " ms.\n";
            PrintProcessTable();
            DeleteTerminatedProcess(PID);
            totalCompletedProcesses++;
        }
    }
    /*This function will release the SSD and make a CORE request. It will also pop any PIDs from the SSD queue if it is not empty.*/
    void SSD_Release(int PID){
        if(!SSDQueue.empty()){
            EventNode* newEventScheduled = new EventNode;
            newEventScheduled->PID = SSDQueue.front();
            SSDQueue.pop();
            newEventScheduled->completionTime = GLOBAL_CLOCK + SSDQueue.front();
            SSDQueue.pop();
            newEventScheduled->interactionType = "non_interactive";
            newEventScheduled->resource = "SSD";
            PushEventToEventQueue(newEventScheduled);
            totalSSDAccesses++;
            IncrementOneToCurrentResourceRequestCounter(newEventScheduled->PID);
        }
        else SSDStatus = true;
        string resourceRequested = ""; int requestedTime = 0;
        GetResourceAndResourceRequestTimeFromNextRequestinInputList(PID,resourceRequested,requestedTime);
        Core_Request(requestedTime,PID,"non_interactive");
    }
    /*This function will release the PID from the User interaction and make a Core request*/
    void UserInteraction_Release(int PID){
        string resourceRequested = ""; int requestedTime = 0;
        GetResourceAndResourceRequestTimeFromNextRequestinInputList(PID,resourceRequested,requestedTime);
        Core_Request(requestedTime,PID,"interactive");
    }
    /*This function will make a core request for Processes that have "START" as their status*/
    void Arrival(int PID){
        string resourceRequested = ""; int requestedTime = 0;
        GetResourceAndResourceRequestTimeFromNextRequestinInputList(PID,resourceRequested,requestedTime);
        Core_Request(requestedTime,PID,"non_interactive");
    }
    /*This function will print the summary of the simulation*/
    void PrintSummary(){
        cout << "SUMMARY:\n";
        cout << "Total elapsed time: " << GLOBAL_CLOCK << " ms\n";
        cout << "Number of processes that completed: " << totalCompletedProcesses << endl;
        cout << "Total number of SSD accesses: " << totalSSDAccesses << endl; 
        cout << "Average number of busy cores: " << totalCORERequestedTime/GLOBAL_CLOCK << endl;
        cout << "SSD utilization: " << totalSSDRequestedTime/GLOBAL_CLOCK << endl;
    }
    /*This is the function that will run the simulation by call the major functions*/
    void RunSimulation(){
        ReadInputFile();
        ScheduleArrivalTimes();
        while(EventListHead != nullptr){
            GLOBAL_CLOCK = EventListHead->completionTime;
            int PID = 0; string resource = ""; int requestedTime = 0;
            TopEventFromQueue(PID,resource,requestedTime);
            PopEventFromEventQueue(); 
            if(resource == "START"){
                cout << "Process " << PID << " starts at time " << GLOBAL_CLOCK << " ms\n"; 
                PrintProcessTable(); 
                AddArrivalProcessToTable(PID);
                Arrival(PID);
            }
            if(resource == "CORE") Core_Release(PID);
            if(resource == "SSD") SSD_Release(PID);
            if(resource == "TTY") UserInteraction_Release(PID);
            SortEventQueue();
        }
        PrintSummary();
    }
};

int main(){
    Simulation* simuPtr = new Simulation;
    simuPtr->RunSimulation();
    return 0;
}
