import { useEffect, useState } from 'react';
import { io } from 'socket.io-client';

type SensorData = {
  sensors: number[]
  speed: {
    left: number,
    right: number
  }
  timestamp: number
};

function App() {
  const [data, setData] = useState<SensorData | null>(null);
  const [socket, setSocket] = useState<any>(null);

  useEffect(() => {
    const socket = io('localhost:5000/', { transports: ['websocket'] });
    setSocket(socket);
    socket.on('connect', () => {
      console.log("connected")
    });
    socket.on('data', (data: SensorData) => {
      // console.log(data)
      setData(data);
    });
    return () => {
      socket.disconnect();
    }

  }, []);


  return (
    <div className="flex w-full justify-center items-center p-10">
      <div className="flex flex-col gap-5">
        <table>
          <tbody>
            <tr>
              <td className="pr-5">Sensor izquierdo:</td>
              <td>{data?.sensors ? data?.sensors[0] : null}</td>
              <td rowSpan={5}>
                <div className='flex flex-row gap-3 px-10'>
                  {data?.sensors?.map((sensorValue, index) => (
                    <div key={index} className={`border border-black w-10 h-10 rounded-full ${sensorValue === 1 ? 'bg-black' : 'bg-white'}`}></div>
                  ))}
                </div>
              </td>
            </tr>
            <tr>
              <td className="pr-5">Sensor medio-izq:</td>
              <td>{data?.sensors[1]}</td>
            </tr>
            <tr>
              <td className="pr-5">Sensor medio:</td>
              <td>{data?.sensors[2]}</td>
            </tr>
            <tr>
              <td className="pr-5">Sensor medio-der:</td>
              <td>{data?.sensors[3]}</td>
            </tr>
            <tr>
              <td className="pr-5">Sensor derecho:</td>
              <td>{data?.sensors[4]}</td>
            </tr>
          </tbody>
        </table>

        <table className="self-start">
          <tbody>
            <tr>
              <td className="pr-5">Velocidad rueda izquierda:</td>
              <td>{data?.speed.left}</td>
            </tr>
            <tr>
              <td className="pr-5">Velocidad rueda derecha:</td>
              <td>{data?.speed.right}</td>
            </tr>
          </tbody>
        </table>
        <div>
          <div>Tiempo: {data?.timestamp}</div>
        </div>

        <div className='flex gap-5'>
          <button className="bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded" onClick={() => { console.log("begin"); socket.emit("begin") }}>
            Begin</button>
          <button className="bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded" onClick={() => { console.log("stop"); socket.emit("stop") }}>
            End</button>
        </div>

      </div>
    </div>
  );
}

export default App;
