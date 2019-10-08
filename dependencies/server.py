from pcaspy import Driver, SimpleServer, Severity
import random
import copy

prefix = "CASC:"

windows = {}
windows['laselock'] = ["LockedA", "LockedB", "SearchA", "SearchB", "InClipA", "InClipB", "HoldA", "HoldB"]
windows['Turbo'] = ["IRTop:StatusCode", "IRTop:ErrorCode", "IRTop:Temperature", "IRTop:Drive", "IRBottom:StatusCode", "IRBottom:ErrorCode", "IRBottom:Temperature", "IRBottom:Drive", "DP:StatusCode", "DP:ErrorCode", "DP:Temperature", "DP:Drive"]

pvdb = {}
for device in ["laselock", "Turbo"]:
    pvdb["{}:SET".format(device)] = {'type' : 'enum', 'enums':['OFF', 'ON']}
    pvdb["{}:IS".format(device)] = {'type': 'enum', 'enums':['OFF', 'ON', 'FAIL'], 'states':[Severity.NO_ALARM, Severity.NO_ALARM, Severity.MINOR_ALARM]}

    for window in windows['Turbo']:
        pvdb["{0}:{1}".format(device, window)] = {'type':'int'}
        
pvdb["laselock:LockedA"] = {'type': 'enum', 'enums':['Notlocked', 'Locked', 'off'], 'states':[Severity.MINOR_ALARM, Severity.NO_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:LockedB"] = {'type': 'enum', 'enums':['Notlocked', 'Locked', 'off'], 'states':[Severity.MINOR_ALARM, Severity.NO_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:SearchA"] = {'type': 'enum', 'enums':['Searchok', 'Searching', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:SearchB"] = {'type': 'enum', 'enums':['Searchok', 'Searching', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:InClipA"] = {'type': 'enum', 'enums':['Inok', 'Inclip', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:InClipB"] = {'type': 'enum', 'enums':['Inok', 'Inclip', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:HoldA"] = {'type': 'enum', 'enums':['Regok', 'Hold', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
pvdb["laselock:HoldB"] = {'type': 'enum', 'enums':['Regok', 'Hold', 'off'], 'states':[Severity.NO_ALARM, Severity.MAJOR_ALARM, Severity.NO_ALARM], 'value':2}
    
    
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