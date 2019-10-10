from pcaspy import Driver, SimpleServer, Severity
import random
import copy

prefix = "CASC:"

pvdb = {}
for device in ["laselock", "Turbo"]:
    pvdb["{}:SET".format(device)] = {'type' : 'enum', 'enums':['OFF', 'ON']}
    pvdb["{}:IS".format(device)] = {'type': 'enum', 'enums':['OFF', 'ON', 'FAIL'], 'states':[Severity.NO_ALARM, Severity.NO_ALARM, Severity.MINOR_ALARM]}
        
pvdb["laselock:LockedA"] = {'type': 'enum', 'enums':['Notlocked', 'Locked', 'off'], 'states':[Severity.MINOR_ALARM, Severity.NO_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:LockedB"] = {'type': 'enum', 'enums':['Notlocked', 'Locked', 'off'], 'states':[Severity.MINOR_ALARM, Severity.NO_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:SearchA"] = {'type': 'enum', 'enums':['Searchok', 'Searching', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:SearchB"] = {'type': 'enum', 'enums':['Searchok', 'Searching', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:InClipA"] = {'type': 'enum', 'enums':['Inok', 'Inclip', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:InClipB"] = {'type': 'enum', 'enums':['Inok', 'Inclip', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:HoldA"] = {'type': 'enum', 'enums':['Regok', 'Hold', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:HoldB"] = {'type': 'enum', 'enums':['Regok', 'Hold', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
    
for name in ["IRTop", "IRBottom", "DP"]:
    pvdb["Turbo:{}:Status".format(name)] = {'type':'enum', 'enums':['off','Stopped','Interlock','Starting','Tuning','Braking','Running','FAULT'], 'states':[Severity.NO_ALARM,Severity.MAJOR_ALARM,Severity.MAJOR_ALARM,Severity.MINOR_ALARM,Severity.MINOR_ALARM,Severity.MAJOR_ALARM,Severity.NO_ALARM,Severity.MAJOR_ALARM]}
    pvdb["Turbo:{}:Error".format(name)] = {'type':'enum', 'enums':['off','ok','FAULT'], 'states':[Severity.NO_ALARM,Severity.NO_ALARM,Severity.MAJOR_ALARM]}
    pvdb["Turbo:{}:Temperature".format(name)] = {'type':'int', 'lolo':20, 'low':25, 'high':31, 'hihi':36, 'hilim':70}#3 degrees either side of normal operating temperature for MINOR_ALARM, 8 degrees for MAJOR_ALARM
    pvdb["Turbo:{}:Drive".format(name)] = {'type':'int', 'lolo':953, 'low':962, 'high':964, 'hihi':973}#1 Hz either side of normal operating speed for MINOR_ALARM, 10 Hz for MAJOR_ALARM
    
    
class myDriver(Driver):
    def  __init__(self):
        super(myDriver, self).__init__()

if __name__ == '__main__':
    server = SimpleServer()
    server.createPV(prefix, pvdb)
    driver = myDriver()

    # process CA transactions
    while True:
        server.process(0.1)