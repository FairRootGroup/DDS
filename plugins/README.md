# For Plug-in developers

## Basic concept

DDS offers a possibility for external developers to make their own RMS plug-ins.

Conceptually, each RMS plug-in is just an executable, which uses a simple DDS plug-in API and is able to deploy and execute a DDS worker package on a corresponding RMS.

The following is a basic workflow:

- User requests to deploy DDS agents or a given RMS using the `dds-submit --rms XXXX` command. Where XXXX is the name of the plug-in user wants to use.
- DDS commander server receives the request, looks for a suitable plug-in (associated with the XXXX name) and starts it. Plug-in has **2 minutes** to connect back to commander to receive exact details about the submit request.
- Once plug-in is started it should contact with the DDS commander server using DDS API, receive details and deploy agents on a given RMS. That's so far it.

## Requirements

- DDS requires each plug-in to have the name according to the following format: `dds-submit-XXXX`, where XXXX is the name of the plug-in (or name of RMS it wraps). All lower case characters.
- A DDS plug-in (executable) and all related files must be sandboxed in a dedicated folder: `path/dds-submit-XXXX/`. The folder path is provided as a commandline argument for all plug-ins. The default location of plug-ins is `$DDS_LOCATION/plugins/dds-submit-XXX/`.
- A DDS plug-in should take two command line arguments  
`[--id arg]`  
and  
`[--path arg]`  
DDS will call the plug-in with this command line arguments and will provide a unique ID and a plug-in directory path. ID must be used when ever plug-in communicates with DDS commander server (see "plug-in-id" in the API section for more info). Plug-in's directory path can be used to access related files if needed.
- Plug-ins are responsible to remove all own temporary files on exit. DDS doesn't take ownership of any file create by plug-ins.

## API

The `dds::intercom_api::CRMSPluginProtocol` is a wrapper class for `plug-in/"DDD commander server"` communication.

Once started and ready the plug-in should subscribe on the `submit` and `message`` command from the DDS commander server.

```cpp
CRMSPluginProtocol prot("plug-in-id");

prot.onSubmit([](const SSubmit& _submit) {
 // Implement submit related functionality here.

 // After submit has completed call stop() function.
 prot.stop();
});


prot.onMessage([](const SMessage& _message) {
 // Message from commander received.
 // Implement related functionality here.
});
```

The `onSubmit` event will deliver to the plugin-in the actual request `dds::intercom_api::SSubmit`.  
It can contain either a configuration file (format of the file is plug-in depended) or simply a number of agents to deploy. But it will always contain the path to the worker package, which plug-in is supposed to deploy on RMS and execute.  
Additionally developers can use a DDS command line tools to find out the location of the worker package: `dds-user-defaults --wrkscript`. This is especially useful when plug-ins use shell scripts.

Once ready the plug-in let's give a hit to DDS commander that we are online and ready for a job:

```cpp
// Let DDS commander know that we are online and start listening for notifications.
prot.start();
```

After that commander will form a submit request and will send it back to the plug-in. By default his call will block the main thread until one of the condition is true:

- 10 minutes timeout,
- failed connection to DDS commander or disconnection from DDS commander,
- explicit call of the `stop()` function.

If you do not want to stop the thread use:

```cpp
// "false" means that we do not block the thread
prot.start(false);
```

If there are no subscribers the thread is not blocked in any case.

Once connected you can use `proto.sendMessage` to send messages. Those messages will be displayed to user while he/she waits on dds-submit command. Be advised, that once commander receives the error message it will forward it to the user and close connection as it means a failed submission.

We strongly recommend to protect `CRMSPluginProtocol` calls in a try/catch block, as all methods can `throw std::exceptions`:

```cpp
try {
 CRMSPluginProtocol prot("plug-in-id");

 prot.onSubmit([](const SSubmit& _submit) {
  // Implement submit related functionality here.

  // report something back to a user
  proto.sendMessage(dds::intercom_api::EMsgSeverity::info, "Text of the info message");

  // After submit has completed call stop() function.
  prot.stop();
 });


 prot.onMessage([](const SMessage& _message) {
  // Message from commander received.
  // Implement related functionality here.
 });

 // Let DDS commander know that we are online and start listening for notifications
 prot.start();
 } catch (exception& _e) {
  // Report error to DDS commander
  proto.sendMessage(dds::intercom_api::EMsgSeverity::error, e.what());
 }
```
