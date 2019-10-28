from pcaspy import Driver, SimpleServer, Severity

#####################################################!!! EDITING may break CASC.exe !!!###########################################################################
prefix = "CASC:"

pvdb = {}
pvdb["AgilisMirrors:SET"] = {'type' : 'enum', 'enums':['OFF', 'ON']}
pvdb["AgilisMirrors:IS"] = {'type': 'enum', 'enums':['OFF', 'ON', 'FAIL'], 'states':[Severity.NO_ALARM, Severity.NO_ALARM, Severity.MINOR_ALARM]}
        
pvdb["AgilisMirrors:StopCommanded"] = {'type':'enum', 'enums':['OK', 'Stop']}
# pvdb["AgilisMirrors:CalibrateCommanded"] = {'type':'int'}
for axis in range(1,9):
    pvdb["AgilisMirrors:Position{}".format(axis)] = {'type':'int'}
    pvdb["AgilisMirrors:Jog{}".format(axis)] = {'type':'int'}

    
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