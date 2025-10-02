import { create } from 'zustand';

import { socket } from '@/utils/socket';

const TrafxStore = (set, get) => ({
    isSocketConnected: false,

    trafixSum: null,
    trafixArr: null,

    setTrafixSum: (data) => set({ trafixSum: data }),
    // pushTrafixArr: (data) => set({ 
    //     trafixArr: data,
    //     trafixSum: {
    //         bits: data.reduce((acc, cur) => acc + cur.bits, 0),
    //         packets: data.reduce((acc, cur) => acc + cur.packets, 0),
    //     },
    // }),
    
    getSocketData: () => {
        const isSocketConnected = get().isSocketConnected;
        if (isSocketConnected) return;

        function handleTrafx(data) {
            console.log(data);
            get().setTrafixSum(data);
        }

        function onConnect() {
            console.log('Connected to server');
        }

        function onDisconnect() {
            console.log('Disconnected from server');
        }

        socket.on('connect', onConnect);
        socket.on('disconnect', onDisconnect);
        socket.on('traffic', handleTrafx);

        return () => {
            socket.off('connect', onConnect);
            socket.off('disconnect', onDisconnect);
            socket.off('traffic');
        };
    },
    
});

const useTrafxStore = create(TrafxStore);

export default useTrafxStore;