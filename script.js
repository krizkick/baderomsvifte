document.addEventListener('DOMContentLoaded', () => {
    const svgElement = document.getElementById('rotatingsvg');
    const container = document.getElementById('viftenivå');
    let duration = 1;

    function updateDuration() {
        const value = parseInt(container.textContent, 10);

        switch (true) {
            case (value >= 1 && value <= 10):
                duration = 5; 
                break;
            case (value >= 11 && value <= 20):
                duration = 4; 
                break;
            case (value >= 21 && value <= 30):
                duration = 3; 
                break;
            case (value >= 31 && value <= 49):
                duration = 2; 
                break;
            case (value >= 50 && value <= 100):
                duration = 1; 
                break;
            default:
                duration = 0; // Stand still
        }

        svgElement.classList.add('paused');
        svgElement.style.animationDuration = `${duration}s`;
        setTimeout(() => {
            svgElement.classList.remove('paused');
        }, 50);
    }

    // Example: Check and update speed every 1 second
    setInterval(updateDuration, 1000);
});

const brokerUrl = 'ws://mqtt.kris2ffer.net:9001'; // WebSocket URL for MQTT
const client = mqtt.connect(brokerUrl);

// Når klienten kobler til
client.on('connect', () => {
    console.log('Connected to MQTT broker');

    // Emner som vi skal abonnere på
    const topics = ['sensor/fuktighet', 'sensor/temperatur', 'sensor/gassmotstand', 'vifte/feedback'];

    // Abonner på hvert emne
    topics.forEach((topic) => {
        client.subscribe(topic, (err) => {
            if (!err) {
                console.log(`Subscribed to topic: ${topic}`);
            } else {
                console.error(`Failed to subscribe to topic: ${topic}`, err);
            }
        });
    });
});

// Når meldinger mottas
client.on('message', (topic, message) => {
    const messageText = message.toString();
    console.log(`Message received on ${topic}: ${messageText}`);

    // Oppdater relevant felt basert på emnet
    switch (topic) {
        case 'sensor/fuktighet':
            updateFuktighet(messageText);
            break;
        case 'sensor/temperatur':
            updateTemperatur(messageText);
            break;
        case 'sensor/gassmotstand':
            updateLuftkvalitet(messageText);
            break;
        case 'vifte/feedback':
            updateVifte(messageText);
            break;
        default:
            console.warn(`Unhandled topic: ${topic}`);
    }
});

// Funksjon for å oppdatere fuktighet
function updateFuktighet(value) {
    const fuktighetElement = document.getElementById("fuktighetVerdi");
    fuktighetElement.textContent = `${value}%`;
}

// Funksjon for å oppdatere temperatur
function updateTemperatur(value) {
    const temperaturElement = document.getElementById("faktiskTemp");
    temperaturElement.textContent = `${value}°C`;
}

// Funksjon for å oppdatere luftkvalitet
function updateLuftkvalitet(value) {
    const luftkvalitetElement = document.getElementById("faktiskLuftKvalitet");
    luftkvalitetElement.textContent = `${value}`;
}

// Funksjon for å oppdatere vifte
function updateVifte(value) {
    const vifteElement = document.getElementById("viftenivå");
    const sliderElement = document.getElementById("vifteSlide");
    vifteElement.textContent = value;
    sliderElement.value = value;
}

// Håndtering av slider for vifte
const slider = document.getElementById("vifteSlide");
const output = document.getElementById("viftenivå");

// Oppdater slider-verdi og send til MQTT
slider.addEventListener("input", () => {
    output.textContent = slider.value;
    client.publish('vifte/hastighet', slider.value);
});
