import tkinter as tk
from tkinter import ttk, messagebox

from bb84_core import run_bb84

try:
    from e91_core import run_e91
except ImportError:
    run_e91 = None


class QKDApp:
    """
    GUI приложение для демонстрации QKD протоколов:
    BB84 и E91.

    Здесь реализованы:
    - Configuration panel
    - Visualization panel
    - Step-by-step mode
    - Real-time statistics display
    """

    def __init__(self, root):
        self.root = root
        self.root.title("SIS4 QKD Simulator: BB84 / E91")
        self.root.geometry("1200x750")

        self.current_result = None
        self.step_index = 0
        self.transmission_log = []

        self.create_layout()

    def create_layout(self):
        """
        Создаёт основной интерфейс:
        слева настройки, по центру визуализация,
        справа статистика.
        """

        main_frame = ttk.Frame(self.root, padding=10)
        main_frame.pack(fill=tk.BOTH, expand=True)

        self.left_frame = ttk.LabelFrame(main_frame, text="Configuration Panel", padding=10)
        self.left_frame.pack(side=tk.LEFT, fill=tk.Y, padx=5)

        self.center_frame = ttk.LabelFrame(main_frame, text="Visualization Panel", padding=10)
        self.center_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)

        self.right_frame = ttk.LabelFrame(main_frame, text="Real-time Statistics", padding=10)
        self.right_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=5)

        self.create_config_panel()
        self.create_visualization_panel()
        self.create_statistics_panel()

    def create_config_panel(self):
        """
        Панель настроек протокола.
        """

        ttk.Label(self.left_frame, text="Protocol:").pack(anchor="w")
        self.protocol_var = tk.StringVar(value="BB84")

        protocol_box = ttk.Combobox(
            self.left_frame,
            textvariable=self.protocol_var,
            values=["BB84", "E91"],
            state="readonly",
            width=18
        )
        protocol_box.pack(anchor="w", pady=5)

        ttk.Label(self.left_frame, text="Number of photons / pairs:").pack(anchor="w")
        self.num_var = tk.IntVar(value=1000)
        ttk.Entry(self.left_frame, textvariable=self.num_var, width=20).pack(anchor="w", pady=5)

        ttk.Label(self.left_frame, text="Check percentage:").pack(anchor="w")
        self.check_var = tk.DoubleVar(value=0.10)
        ttk.Entry(self.left_frame, textvariable=self.check_var, width=20).pack(anchor="w", pady=5)

        ttk.Label(self.left_frame, text="Error threshold:").pack(anchor="w")
        self.threshold_var = tk.DoubleVar(value=0.11)
        ttk.Entry(self.left_frame, textvariable=self.threshold_var, width=20).pack(anchor="w", pady=5)

        ttk.Label(self.left_frame, text="Noise rate:").pack(anchor="w")
        self.noise_var = tk.DoubleVar(value=0.01)
        ttk.Entry(self.left_frame, textvariable=self.noise_var, width=20).pack(anchor="w", pady=5)

        self.eve_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(
            self.left_frame,
            text="Enable Eve",
            variable=self.eve_var
        ).pack(anchor="w", pady=10)

        ttk.Button(
            self.left_frame,
            text="Run Simulation",
            command=self.run_simulation
        ).pack(fill=tk.X, pady=5)

        ttk.Button(
            self.left_frame,
            text="Next Step",
            command=self.next_step
        ).pack(fill=tk.X, pady=5)

        ttk.Button(
            self.left_frame,
            text="Reset Step",
            command=self.reset_step
        ).pack(fill=tk.X, pady=5)

        ttk.Button(
            self.left_frame,
            text="Clear",
            command=self.clear_all
        ).pack(fill=tk.X, pady=5)

        ttk.Label(
            self.left_frame,
            text="\nTip:\nUse BB84 for full step-by-step.\nE91 shows final statistics.",
            foreground="gray"
        ).pack(anchor="w", pady=10)

    def create_visualization_panel(self):
        """
        Центральная визуализация:
        Alice → Eve → Bob
        """

        self.canvas = tk.Canvas(
            self.center_frame,
            width=650,
            height=380,
            bg="white"
        )
        self.canvas.pack(fill=tk.BOTH, expand=True)

        self.step_text = tk.Text(
            self.center_frame,
            height=12,
            wrap=tk.WORD
        )
        self.step_text.pack(fill=tk.X, pady=10)

        self.draw_empty_scene()

    def create_statistics_panel(self):
        """
        Правая панель статистики.
        """

        self.stats_text = tk.Text(
            self.right_frame,
            width=38,
            height=35,
            wrap=tk.WORD
        )
        self.stats_text.pack(fill=tk.BOTH, expand=True)

        self.update_stats_text("No simulation yet.")

    def draw_empty_scene(self):
        """
        Рисует пустую сцену Alice → Eve → Bob.
        """

        self.canvas.delete("all")

        self.canvas.create_text(100, 80, text="Alice", font=("Arial", 18, "bold"))
        self.canvas.create_oval(60, 110, 140, 190, fill="#dceeff")
        self.canvas.create_text(100, 150, text="A", font=("Arial", 20, "bold"))

        self.canvas.create_text(325, 80, text="Eve", font=("Arial", 18, "bold"))
        self.canvas.create_oval(285, 110, 365, 190, fill="#ffe0e0")
        self.canvas.create_text(325, 150, text="E", font=("Arial", 20, "bold"))

        self.canvas.create_text(550, 80, text="Bob", font=("Arial", 18, "bold"))
        self.canvas.create_oval(510, 110, 590, 190, fill="#e3ffe0")
        self.canvas.create_text(550, 150, text="B", font=("Arial", 20, "bold"))

        self.canvas.create_line(145, 150, 280, 150, arrow=tk.LAST, width=3)
        self.canvas.create_line(370, 150, 505, 150, arrow=tk.LAST, width=3)

        self.canvas.create_text(
            325,
            270,
            text="Run simulation, then use Next Step",
            font=("Arial", 14)
        )

    def run_simulation(self):
        """
        Запускает выбранный протокол.
        """

        try:
            protocol = self.protocol_var.get()
            num = self.num_var.get()
            check_percentage = self.check_var.get()
            threshold = self.threshold_var.get()
            eve_enabled = self.eve_var.get()
            noise_rate = self.noise_var.get()

            if protocol == "BB84":
                self.current_result = run_bb84(
                    num_photons=num,
                    check_percentage=check_percentage,
                    error_threshold=threshold,
                    eve_enabled=eve_enabled,
                    noise_rate=noise_rate
                )

                self.transmission_log = self.current_result.get("transmission_log", [])

            elif protocol == "E91":
                if run_e91 is None:
                    messagebox.showerror("Error", "e91_core.py not found.")
                    return

                self.current_result = run_e91(
                    num_pairs=num,
                    check_percentage=check_percentage,
                    error_threshold=threshold,
                    eve_enabled=eve_enabled,
                    noise_rate=noise_rate
                )

                self.transmission_log = []

            self.step_index = 0
            self.update_statistics_from_result()
            self.draw_summary_scene()
            self.show_summary_text()

        except Exception as error:
            messagebox.showerror("Simulation Error", str(error))

    def update_statistics_from_result(self):
        """
        Обновляет панель статистики по результату.
        """

        if not self.current_result:
            return

        result = self.current_result

        lines = []
        lines.append(f"Protocol: {result.get('protocol')}")
        lines.append(f"Status: {result.get('status')}")
        lines.append("")
        lines.append(f"Initial count: {result.get('initial_count')}")
        lines.append(f"Sifted key length: {result.get('sifted_key_length')}")
        lines.append(f"Checked bits: {result.get('checked_bits')}")
        lines.append(f"Errors found: {result.get('errors_found', 'N/A')}")

        error_rate = result.get("error_rate", 0)
        lines.append(f"Error rate: {error_rate:.2%}")

        if "basis_match_rate" in result:
            lines.append(f"Basis match rate: {result.get('basis_match_rate'):.2%}")

        lines.append(f"Final key length: {result.get('final_key_length')}")
        lines.append(f"Efficiency: {result.get('efficiency'):.2f}%")
        lines.append(f"Eve enabled: {result.get('eve_enabled')}")
        lines.append(f"Noise rate: {result.get('noise_rate', 0):.2%}")

        if "chsh_s" in result:
            lines.append("")
            lines.append(f"CHSH S value: {result.get('chsh_s')}")

        if "keys_match_before_amplification" in result:
            lines.append("")
            lines.append(f"Keys before amplification match:")
            lines.append(str(result.get("keys_match_before_amplification")))

        if "final_keys_match" in result:
            lines.append(f"Final keys match:")
            lines.append(str(result.get("final_keys_match")))

        if result.get("final_key"):
            lines.append("")
            lines.append("Final key preview:")
            lines.append(result.get("final_key", "")[:64])

        if result.get("eve_enabled"):
            lines.append("")
            lines.append("--- Eve Statistics ---")
            for key, value in result.items():
                if key.startswith("eve_"):
                    lines.append(f"{key}: {value}")

        self.update_stats_text("\n".join(lines))

    def update_stats_text(self, text):
        """
        Обновляет текст статистики.
        """

        self.stats_text.delete("1.0", tk.END)
        self.stats_text.insert(tk.END, text)

    def draw_summary_scene(self):
        """
        Рисует итоговую сцену после запуска.
        """

        self.draw_empty_scene()

        if not self.current_result:
            return

        result = self.current_result
        status = result.get("status")

        if result.get("eve_enabled"):
            self.canvas.create_text(
                325,
                220,
                text="Eve is active: intercept-resend attack",
                font=("Arial", 13, "bold"),
                fill="red"
            )
        else:
            self.canvas.create_text(
                325,
                220,
                text="No Eve: normal quantum channel",
                font=("Arial", 13, "bold"),
                fill="green"
            )

        if result.get("success"):
            color = "green"
            message = "SECURE KEY ESTABLISHED"
        else:
            color = "red"
            message = "PROTOCOL ABORTED / INSECURE"

        self.canvas.create_rectangle(170, 290, 480, 350, outline=color, width=3)
        self.canvas.create_text(
            325,
            320,
            text=f"{message}\nStatus: {status}",
            font=("Arial", 14, "bold"),
            fill=color
        )

    def show_summary_text(self):
        """
        Показывает краткое текстовое объяснение после запуска.
        """

        self.step_text.delete("1.0", tk.END)

        if not self.current_result:
            return

        result = self.current_result

        text = []
        text.append("Simulation completed.\n")
        text.append(f"Protocol: {result.get('protocol')}\n")
        text.append(f"Status: {result.get('status')}\n\n")

        if result.get("protocol") == "BB84":
            text.append("BB84 phases:\n")
            text.append("1. Alice generated random bits and bases.\n")
            text.append("2. Bob measured photons in random bases.\n")
            text.append("3. Alice and Bob compared only bases.\n")
            text.append("4. Matching bases formed the sifted key.\n")
            text.append("5. Error checking estimated possible eavesdropping.\n")
            text.append("6. Privacy amplification produced final key.\n\n")
            text.append("Use Next Step to inspect individual photon transmissions.\n")
        else:
            text.append("E91 bonus simulation completed.\n")
            text.append("E91 uses entangled pairs and CHSH/Bell correlation checking.\n")

        self.step_text.insert(tk.END, "".join(text))

    def next_step(self):
        """
        Показывает следующий шаг передачи фотона.
        Работает для BB84, потому что run_bb84 возвращает transmission_log.
        """

        if not self.current_result:
            messagebox.showinfo("Info", "Run simulation first.")
            return

        if self.current_result.get("protocol") != "BB84":
            messagebox.showinfo("Info", "Step-by-step mode is implemented for BB84.")
            return

        if not self.transmission_log:
            messagebox.showinfo(
                "Info",
                "No transmission log found. Make sure bb84_core.py returns transmission_log."
            )
            return

        if self.step_index >= len(self.transmission_log):
            messagebox.showinfo("Info", "No more saved steps. Only first 50 are stored.")
            return

        step = self.transmission_log[self.step_index]
        self.draw_step(step)
        self.show_step_text(step)

        self.step_index += 1

    def draw_step(self, step: dict):
        """
        Рисует один шаг передачи фотона.
        """

        self.draw_empty_scene()

        alice_bit = step["alice_bit"]
        alice_basis = step["alice_basis"]
        alice_polarization = step["alice_polarization"]
        bob_basis = step["bob_basis"]
        bob_result = step["bob_result"]
        basis_matched = step["basis_matched"]

        # Рисуем фотон как маленький круг
        self.canvas.create_oval(300, 135, 350, 185, fill="#fff3b0", outline="black", width=2)
        self.canvas.create_text(325, 160, text="γ", font=("Arial", 22, "bold"))

        # Alice details
        self.canvas.create_text(
            100,
            230,
            text=f"bit={alice_bit}\nbasis={alice_basis}\npol={alice_polarization}°",
            font=("Arial", 12)
        )

        # Bob details
        self.canvas.create_text(
            550,
            230,
            text=f"basis={bob_basis}\nresult={bob_result}",
            font=("Arial", 12)
        )

        if self.current_result.get("eve_enabled"):
            eve_text = "Eve intercepts\nand resends"
            eve_color = "red"
        else:
            eve_text = "No Eve"
            eve_color = "green"

        self.canvas.create_text(
            325,
            230,
            text=eve_text,
            font=("Arial", 12, "bold"),
            fill=eve_color
        )

        if basis_matched:
            result_text = "Bases matched → deterministic result"
            result_color = "green"
        else:
            result_text = "Bases different → random result"
            result_color = "orange"

        self.canvas.create_rectangle(150, 300, 500, 360, outline=result_color, width=3)
        self.canvas.create_text(
            325,
            330,
            text=result_text,
            font=("Arial", 14, "bold"),
            fill=result_color
        )

    def show_step_text(self, step: dict):
        """
        Текстовое объяснение одного шага.
        """

        self.step_text.delete("1.0", tk.END)

        text = []
        text.append(f"Step #{step['index'] + 1}\n\n")
        text.append(f"Alice bit: {step['alice_bit']}\n")
        text.append(f"Alice basis: {step['alice_basis']}\n")
        text.append(f"Alice polarization: {step['alice_polarization']}°\n\n")
        text.append(f"Bob basis: {step['bob_basis']}\n")
        text.append(f"Bob measured result: {step['bob_result']}\n\n")

        if step["basis_matched"]:
            text.append("Explanation:\n")
            text.append("Alice and Bob used the same basis, so Bob's result is deterministic.\n")
        else:
            text.append("Explanation:\n")
            text.append("Alice and Bob used different bases, so Bob's result is random.\n")

        self.step_text.insert(tk.END, "".join(text))

    def reset_step(self):
        """
        Сбрасывает step-by-step режим.
        """

        self.step_index = 0
        self.draw_summary_scene()
        self.show_summary_text()

    def clear_all(self):
        """
        Очищает интерфейс.
        """

        self.current_result = None
        self.step_index = 0
        self.transmission_log = []

        self.draw_empty_scene()
        self.update_stats_text("No simulation yet.")
        self.step_text.delete("1.0", tk.END)


def main():
    root = tk.Tk()
    app = QKDApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()