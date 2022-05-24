from pyModbusTCP.client import ModbusClient

def main():
    c = ModbusClient(host="192.168.1.10", port=5002, unit_id=1, auto_open=True)

    regs = c.read_holding_registers(0,2)
    if regs:
        print(regs)
    else:
        print("read error")


main()
