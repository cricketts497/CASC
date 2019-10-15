from pcaspy import Driver, SimpleServer, Severity
import random
import copy

#####################################################!!! EDITING may break CASC.exe !!!###########################################################################
prefix = "CASC:"

pvdb = {}
for device in ["Laselock", "Turbo", "BL", "Heinzinger30k", "Heinzinger20k"]:
    pvdb["{}:SET".format(device)] = {'type' : 'enum', 'enums':['OFF', 'ON']}
    pvdb["{}:IS".format(device)] = {'type': 'enum', 'enums':['OFF', 'ON', 'FAIL'], 'states':[Severity.NO_ALARM, Severity.NO_ALARM, Severity.MINOR_ALARM]}
        
#laselock device
pvdb["Laselock:LockedA"] = {'type': 'enum', 'enums':['Notlocked', 'Locked', 'off'], 'states':[Severity.MINOR_ALARM, Severity.NO_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["Laselock:LockedB"] = {'type': 'enum', 'enums':['Notlocked', 'Locked', 'off'], 'states':[Severity.MINOR_ALARM, Severity.NO_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["Laselock:SearchA"] = {'type': 'enum', 'enums':['Searchok', 'Searching', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["Laselock:SearchB"] = {'type': 'enum', 'enums':['Searchok', 'Searching', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["Laselock:InClipA"] = {'type': 'enum', 'enums':['Inok', 'Inclip', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["Laselock:InClipB"] = {'type': 'enum', 'enums':['Inok', 'Inclip', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["Laselock:HoldA"] = {'type': 'enum', 'enums':['Regok', 'Hold', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["Laselock:HoldB"] = {'type': 'enum', 'enums':['Regok', 'Hold', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
    
#agilent TV301 turbo devices
for name in ["IRTop", "IRBottom", "DP"]:
    pvdb["Turbo:{}:Status".format(name)] = {'type':'enum', 'enums':['off','Stopped','Interlock','Starting','Tuning','Braking','Running','FAULT'], 'states':[Severity.NO_ALARM,Severity.MAJOR_ALARM,Severity.MAJOR_ALARM,Severity.MINOR_ALARM,Severity.MINOR_ALARM,Severity.MAJOR_ALARM,Severity.NO_ALARM,Severity.MAJOR_ALARM]}
    pvdb["Turbo:{}:Error".format(name)] = {'type':'enum', 'enums':['off','ok','FAULT'], 'states':[Severity.NO_ALARM,Severity.NO_ALARM,Severity.MAJOR_ALARM]}
    pvdb["Turbo:{}:Temperature".format(name)] = {'type':'int', 'lolo':20, 'low':25, 'high':31, 'hihi':36, 'hilim':70, 'unit':'C'}#3 degrees either side of normal operating temperature for MINOR_ALARM, 8 degrees for MAJOR_ALARM
    pvdb["Turbo:{}:Drive".format(name)] = {'type':'int', 'lolo':953, 'low':962, 'high':964, 'hihi':973, 'unit':'Hz'}#1 Hz either side of normal operating speed for MINOR_ALARM, 10 Hz for MAJOR_ALARM
    
#nxds pump backing line devices
for name in ["20MT", "20Ebara", "IR", "DP", "QT"]:
    pvdb["BL:{}:Status".format(name)] = {'type':'enum', 'enums':['off','Running','Decelerating','Standby','PumpOff','WARNING','FAULT'], 'states':[Severity.NO_ALARM,Severity.NO_ALARM,Severity.MAJOR_ALARM,Severity.MAJOR_ALARM,Severity.MAJOR_ALARM,Severity.MAJOR_ALARM,Severity.MAJOR_ALARM]}
    pvdb["BL:{}:Service".format(name)] = {'type':'enum', 'enums':['off','ok','Service'], 'states':[Severity.NO_ALARM,Severity.NO_ALARM,Severity.MAJOR_ALARM,Severity.MAJOR_ALARM]}
    pvdb["BL:{}:Speed".format(name)] = {'type':'int', 'lolo':25, 'low':29, 'high':31, 'hihi':35, 'unit':'Hz'}
    pvdb["BL:{}:Temperature".format(name)] = {'type':'int', 'lolo':20, 'low':45, 'high':55, 'hihi':60, 'unit':'C'}

#Heinzinger30k device
pvdb["Heinzinger30k:VoltageSetpoint"] = {'type':'float', 'low':-1, 'high':30001, 'prec':1, 'unit':'V'}
pvdb["Heinzinger30k:CurrentSetpoint"] = {'type':'float', 'low':-1, 'high':1, 'hihi':3, 'prec':2, 'unit':'mA'}
pvdb["Heinzinger30k:OutputSetpoint"] = {'type':'enum', 'enums':['OutputOff','OutputOn','off'], 'value':2}
pvdb["Heinzinger30k:VoltageApplied"] = {'type':'float', 'low':-1, 'high':30001, 'prec':2, 'unit':'V'}
pvdb["Heinzinger30k:VoltageCommanded"] = {'type':'int', 'low':-1, 'high':30001, 'unit':'V'}
pvdb["Heinzinger30k:CurrentCommanded"] = {'type':'float', 'low':-1, 'high':1, 'hihi':3, 'prec':2, 'unit':'mA'}
pvdb["Heinzinger30k:OutputCommanded"] = {'type':'enum', 'enums':['OutputOff','OutputOn']}

#Heinzinger20k device
pvdb["Heinzinger20k:VoltageSetpoint"] = {'type':'float', 'low':-1, 'high':20001, 'prec':1, 'unit':'V'}
pvdb["Heinzinger20k:CurrentSetpoint"] = {'type':'float', 'low':-1, 'high':1, 'hihi':3, 'prec':2, 'unit':'mA'}
pvdb["Heinzinger20k:OutputSetpoint"] = {'type':'enum', 'enums':['OutputOff','OutputOn','off'], 'value':2}
pvdb["Heinzinger20k:VoltageApplied"] = {'type':'float', 'low':-1,'high':20001, 'prec':2, 'unit':'V'}
pvdb["Heinzinger20k:VoltageCommanded"] = {'type':'int', 'low':-1, 'high':20001, 'unit':'V'}
pvdb["Heinzinger20k:CurrentCommanded"] = {'type':'float', 'low':-1,'high':1, 'hihi':3, 'prec':2, 'unit':'mA'}
pvdb["Heinzinger20k:OutputCommanded"] = {'type':'enum', 'enums':['OutputOff','OutputOn']}
    
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
######################################################################################################################################################################