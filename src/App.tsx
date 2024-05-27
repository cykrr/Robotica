import { useEffect, useState } from 'react';

type SensorData = {
  sensors: number[]
  speed: {
    left: number,
    right: number
  }
  timestamp: number
};

function App() {
  const [data, setData] = useState<SensorData>({sensors: [0,0,0,0,0], speed: {left: 0, right: 0}, timestamp: 0});

  useEffect(() => {
    async function fetchFile() {
      try {
        const response = await fetch('http://localhost:5000/data');
        const content = await response.json()
        setData(content);
      } catch (error) {
        console.error('Error al leer el archivo:', error);
      }
    }

    fetchFile();
    
    const intervalId = setInterval(fetchFile, 500);
    return () => clearInterval(intervalId);
  }, []);


  return (
    <div className="flex w-full justify-center items-center p-10">
      <div className="flex flex-col gap-5">
        <table>
          <tbody>
            <tr>
              <td className="pr-5">Sensor izquierdo:</td>
              <td>{data?.sensors[0]}</td>
              <td rowSpan={5}>
                <div className='flex flex-row gap-3 px-10'>
                  {data?.sensors.map((sensorValue, index) => (
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
      </div>
    </div>
  );
}

export default App;
