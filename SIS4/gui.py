import tkinter as tk
from tkinter import ttk, messagebox

from bb84_core import run_bb84

try:
    from e91_core import run_e91
except ImportError:
    run_e91 = None


POLARIZATION_SYMBOLS = {
    0: "↔",
    45: "⤢",
    90: "↕",
    135: "⤡",
}


class QKDApp:
    """
    GUI приложение для демонстрации QKD протоколов:
    BB84 и E91.

    Реализовано:
    - Configuration panel
    - Visualization panel
    - Step-by-step mode
    - Auto Play / Pause
    - Key evolution visualization
    - Real-time statistics display
    """

    def __init__(self, root):
        self.root = root
        self.root.title("SIS4 QKD Simulator: BB84 / E91")
        self.root.geometry("1250x780")

        self.current_result = None
        self.step_index = 0
        self.transmission_log = []
        self.auto_play_active = False
        self.animation_speed_ms = 900

        self.create_layout()

    def create_layout(self):
        """Создаёт основной интерфейс."""

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
        """Панель настроек протокола."""

        ttk.Label(self.left_frame, text="Protocol:").pack(anchor="w")
        self.protocol_var = tk.StringVar(value="BB84")

        ttk.Combobox(
            self.left_frame,
            textvariable=self.protocol_var,
            values=["BB84", "E91"],
            state="readonly",
            width=18
        ).pack(anchor="w", pady=5)

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
            text="Auto Play",
            command=self.start_auto_play
        ).pack(fill=tk.X, pady=5)

        ttk.Button(
            self.left_frame,
            text="Pause",
            command=self.pause_auto_play
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
            text="\nBB84: full step-by-step mode\nE91: final statistics + CHSH",
            foreground="gray"
        ).pack(anchor="w", pady=10)

    def create_visualization_panel(self):
        """Создаёт canvas и текстовое объяснение шагов."""

        self.canvas = tk.Canvas(
            self.center_frame,
            width=700,
            height=430,
            bg="white"
        )
        self.canvas.pack(fill=tk.BOTH, expand=True)

        self.step_text = tk.Text(
            self.center_frame,
            height=13,
            wrap=tk.WORD
        )
        self.step_text.pack(fill=tk.X, pady=10)

        self.draw_empty_scene()

    def create_statistics_panel(self):
        """Создаёт правую панель статистики."""

        self.stats_text = tk.Text(
            self.right_frame,
            width=42,
            height=38,
            wrap=tk.WORD
        )
        self.stats_text.pack(fill=tk.BOTH, expand=True)

        self.update_stats_text("No simulation yet.")

    def draw_empty_scene(self, show_hint: bool = True):
        """Рисует базовую сцену Alice → Eve → Bob."""

        self.canvas.delete("all")

        self.canvas.create_text(100, 55, text="Alice", font=("Arial", 18, "bold"))
        self.canvas.create_oval(60, 85, 140, 165, fill="#dceeff")
        self.canvas.create_text(100, 125, text="A", font=("Arial", 20, "bold"))

        self.canvas.create_text(350, 55, text="Eve", font=("Arial", 18, "bold"))
        self.canvas.create_oval(310, 85, 390, 165, fill="#ffe0e0")
        self.canvas.create_text(350, 125, text="E", font=("Arial", 20, "bold"))

        self.canvas.create_text(600, 55, text="Bob", font=("Arial", 18, "bold"))
        self.canvas.create_oval(560, 85, 640, 165, fill="#e3ffe0")
        self.canvas.create_text(600, 125, text="B", font=("Arial", 20, "bold"))

        self.canvas.create_line(145, 125, 305, 125, arrow=tk.LAST, width=3)
        self.canvas.create_line(395, 125, 555, 125, arrow=tk.LAST, width=3)

        # Этот текст показываем только на пустом экране,
        # чтобы он не накладывался на step-by-step текст.
        if show_hint:
            self.canvas.create_text(
                350,
                215,
                text="Run simulation, then use Next Step / Auto Play",
                font=("Arial", 14)
            )

    def run_simulation(self):
        """Запускает выбранный протокол."""

        try:
            protocol = self.protocol_var.get()
            num = self.num_var.get()
            check_percentage = self.check_var.get()
            threshold = self.threshold_var.get()
            eve_enabled = self.eve_var.get()
            noise_rate = self.noise_var.get()

            self.pause_auto_play()

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
        """Обновляет панель статистики по результату."""

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
            lines.append(f"Keys before amplification match: {result.get('keys_match_before_amplification')}")

        if "final_keys_match" in result:
            lines.append(f"Final keys match: {result.get('final_keys_match')}")

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
        """Обновляет текст статистики."""

        self.stats_text.delete("1.0", tk.END)
        self.stats_text.insert(tk.END, text)

    def draw_summary_scene(self):
        """Рисует итоговую сцену после запуска."""

        self.draw_empty_scene()

        if not self.current_result:
            return

        result = self.current_result
        status = result.get("status")

        if result.get("eve_enabled"):
            self.canvas.create_text(
                350,
                185,
                text="Eve is active: intercept-resend attack",
                font=("Arial", 13, "bold"),
                fill="red"
            )
        else:
            self.canvas.create_text(
                350,
                185,
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

        self.canvas.create_rectangle(190, 240, 510, 300, outline=color, width=3)
        self.canvas.create_text(
            350,
            270,
            text=f"{message}\nStatus: {status}",
            font=("Arial", 14, "bold"),
            fill=color
        )

        self.draw_key_evolution(result)

    def draw_key_evolution(self, result: dict):
        """
        Рисует key evolution:
        initial → sifted → after checking → final.
        """

        initial = result.get("initial_count", 0)
        sifted = result.get("sifted_key_length", 0)
        checked = result.get("checked_bits", 0)
        after_checking = max(0, sifted - checked)
        final_key = result.get("final_key_length", 0)

        values = [
            ("Initial", initial),
            ("Sifted", sifted),
            ("After check", after_checking),
            ("Final", final_key),
        ]

        max_value = max([value for _, value in values] + [1])

        self.canvas.create_text(
            350,
            335,
            text="Key Evolution",
            font=("Arial", 14, "bold")
        )

        start_x = 110
        bar_y = 375
        max_height = 70
        bar_width = 90
        gap = 60

        for index, (label, value) in enumerate(values):
            x1 = start_x + index * (bar_width + gap)
            x2 = x1 + bar_width
            height = int((value / max_value) * max_height)
            y1 = bar_y - height
            y2 = bar_y

            self.canvas.create_rectangle(x1, y1, x2, y2, fill="#cfe8ff", outline="black")
            self.canvas.create_text((x1 + x2) // 2, y1 - 12, text=str(value), font=("Arial", 10))
            self.canvas.create_text((x1 + x2) // 2, y2 + 15, text=label, font=("Arial", 10))

    def show_summary_text(self):
        """Показывает краткое объяснение после запуска."""

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
            text.append("Use Next Step or Auto Play to inspect photon transmissions.\n")
        else:
            text.append("E91 bonus simulation completed.\n")
            text.append("E91 uses entangled pairs and CHSH/Bell correlation checking.\n")

        self.step_text.insert(tk.END, "".join(text))

    def next_step(self):
        """Показывает следующий шаг передачи фотона."""

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
            self.pause_auto_play()
            messagebox.showinfo("Info", "No more saved steps. Only first 50 are stored.")
            return

        step = self.transmission_log[self.step_index]
        self.draw_step(step)
        self.show_step_text(step)
        self.step_index += 1

    def start_auto_play(self):
        """Запускает автоматический step-by-step режим."""

        if not self.current_result:
            messagebox.showinfo("Info", "Run simulation first.")
            return

        if self.current_result.get("protocol") != "BB84":
            messagebox.showinfo("Info", "Auto Play is implemented for BB84.")
            return

        self.auto_play_active = True
        self.auto_play_step()

    def auto_play_step(self):
        """Один шаг автоматической анимации."""

        if not self.auto_play_active:
            return

        if self.step_index >= len(self.transmission_log):
            self.auto_play_active = False
            return

        self.next_step()
        self.root.after(self.animation_speed_ms, self.auto_play_step)

    def pause_auto_play(self):
        """Ставит Auto Play на паузу."""

        self.auto_play_active = False

    def draw_step(self, step: dict):
        """Рисует один шаг передачи фотона."""

        self.draw_empty_scene()

        alice_bit = step["alice_bit"]
        alice_basis = step["alice_basis"]
        alice_polarization = step["alice_polarization"]
        bob_basis = step["bob_basis"]
        bob_result = step["bob_result"]
        basis_matched = step["basis_matched"]

        symbol = POLARIZATION_SYMBOLS.get(alice_polarization, "?")

        # Визуальный фотон
        photon_x = 350
        self.canvas.create_oval(photon_x - 25, 100, photon_x + 25, 150, fill="#fff3b0", outline="black", width=2)
        self.canvas.create_text(photon_x, 125, text=symbol, font=("Arial", 22, "bold"))

        self.canvas.create_text(
            100,
            210,
            text=f"bit={alice_bit}\nbasis={alice_basis}\npol={alice_polarization}° {symbol}",
            font=("Arial", 12)
        )

        self.canvas.create_text(
            600,
            210,
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
            350,
            210,
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

        self.canvas.create_rectangle(160, 270, 540, 330, outline=result_color, width=3)
        self.canvas.create_text(
            350,
            300,
            text=result_text,
            font=("Arial", 14, "bold"),
            fill=result_color
        )

        self.canvas.create_text(
            350,
            365,
            text=f"Step {self.step_index + 1}/{len(self.transmission_log)}",
            font=("Arial", 12)
        )

    def show_step_text(self, step: dict):
        """Текстовое объяснение одного шага."""

        self.step_text.delete("1.0", tk.END)

        symbol = POLARIZATION_SYMBOLS.get(step["alice_polarization"], "?")

        text = []
        text.append(f"Step #{step['index'] + 1}\n\n")
        text.append(f"Alice bit: {step['alice_bit']}\n")
        text.append(f"Alice basis: {step['alice_basis']}\n")
        text.append(f"Alice polarization: {step['alice_polarization']}° {symbol}\n\n")
        text.append(f"Bob basis: {step['bob_basis']}\n")
        text.append(f"Bob measured result: {step['bob_result']}\n\n")

        if self.current_result.get("eve_enabled"):
            text.append("Eve is enabled, so the photon may be disturbed before Bob measures it.\n\n")

        if step["basis_matched"]:
            text.append("Explanation:\n")
            text.append("Alice and Bob used the same basis, so Bob's result is deterministic.\n")
        else:
            text.append("Explanation:\n")
            text.append("Alice and Bob used different bases, so Bob's result is random.\n")

        self.step_text.insert(tk.END, "".join(text))

    def reset_step(self):
        """Сбрасывает step-by-step режим."""

        self.pause_auto_play()
        self.step_index = 0
        self.draw_summary_scene()
        self.show_summary_text()

    def clear_all(self):
        """Очищает интерфейс."""

        self.pause_auto_play()
        self.current_result = None
        self.step_index = 0
        self.transmission_log = []

        self.draw_empty_scene()
        self.update_stats_text("No simulation yet.")
        self.step_text.delete("1.0", tk.END)


def main():
    root = tk.Tk()
    QKDApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()